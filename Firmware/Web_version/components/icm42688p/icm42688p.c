#include "icm42688p.h"
#include "esp_log.h"
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define TAG "ICM42688"
#define ICM42688_ADDRESS 0x69
#define WHO_AM_I_REG 0x75
#define EXPECTED_WHO_AM_I 0x47
#define ACC_SENSITIVITY (4.0f / 32768.0f)
#define GYRO_SENSITIVITY (1000.0f / 32768.0f)
#define RAD2DEG 57.2957795f

static i2c_port_t i2c_port;
static float g_roll=0, g_pitch=0, g_yaw=0;
static float gyro_bias_x=0, gyro_bias_y=0, gyro_bias_z=0;
static int gyro_calibrated=0;
static int64_t last_time_us=0;

// 内部函数声明
static void ICM42688_CalibrateGyro(void);
static void i2c_master_init(i2c_port_t port, int sda, int scl);
static void writeReg(uint8_t sub, uint8_t data);
static uint8_t readReg(uint8_t sub);
static void readRegs(uint8_t sub, uint8_t count, uint8_t *data);

// ---------------------- 初始化函数 ----------------------
esp_err_t ICM42688_Init(i2c_port_t port, int sda_gpio, int scl_gpio)
{
    i2c_port = port;
    i2c_master_init(port, sda_gpio, scl_gpio);

    // 检查 WHO_AM_I
    uint8_t who = readReg(WHO_AM_I_REG);
    if (who != EXPECTED_WHO_AM_I) {
        ESP_LOGE(TAG,"WHO_AM_I check failed: 0x%02X", who);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"ICM42688 detected");

    // 配置寄存器（示例）
    writeReg(0x76, 0); // BANK_SEL=0
    writeReg(0x11, 0x01); // DEVICE_CONFIG 软复位
    vTaskDelay(pdMS_TO_TICKS(100));
    writeReg(0x50, 0x48); // ACC_CONFIG0 ±4g, 100Hz
    writeReg(0x4F, 0x28); // GYRO_CONFIG0 ±1000dps, 100Hz
    writeReg(0x4E, 0x0f); // PWR_MGMT0 LN mode
    vTaskDelay(pdMS_TO_TICKS(100));

    // 陀螺标定
    ICM42688_CalibrateGyro();

    return ESP_OK;
}

// ---------------------- I2C 基础函数 ----------------------
static void i2c_master_init(i2c_port_t port, int sda, int scl) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(port, &conf);
    i2c_driver_install(port, conf.mode, 0, 0, 0);
}

static void writeReg(uint8_t sub, uint8_t data) {
    uint8_t buf[2] = {sub, data};
    i2c_master_write_to_device(i2c_port, ICM42688_ADDRESS, buf, 2, 1000 / portTICK_PERIOD_MS);
}

static uint8_t readReg(uint8_t sub) {
    uint8_t data = 0;
    i2c_master_write_read_device(i2c_port, ICM42688_ADDRESS, &sub, 1, &data, 1, 1000 / portTICK_PERIOD_MS);
    return data;
}

static void readRegs(uint8_t sub, uint8_t count, uint8_t *data) {
    i2c_master_write_read_device(i2c_port, ICM42688_ADDRESS, &sub, 1, data, count, 1000 / portTICK_PERIOD_MS);
}

// ---------------------- 获取传感器 ----------------------
void ICM42688_getacc(int16_t *ax,int16_t *ay,int16_t *az) {
    uint8_t buf[6];
    readRegs(0x1F, 6, buf);
    *ax = (buf[0]<<8)|buf[1];
    *ay = (buf[2]<<8)|buf[3];
    *az = (buf[4]<<8)|buf[5];
}

void ICM42688_getgyro(int16_t *gx,int16_t *gy,int16_t *gz) {
    uint8_t buf[6];
    readRegs(0x25,6,buf);
    *gx = (buf[0]<<8)|buf[1];
    *gy = (buf[2]<<8)|buf[3];
    *gz = (buf[4]<<8)|buf[5];
}

// ---------------------- 姿态解算 ----------------------
void ICM42688_AttitudeUpdate(Attitude_t *att) {
    int16_t ax, ay, az, gx, gy, gz;
    ICM42688_getacc(&ax,&ay,&az);
    ICM42688_getgyro(&gx,&gy,&gz);

    float f_ax = ax*ACC_SENSITIVITY;
    float f_ay = ay*ACC_SENSITIVITY;
    float f_az = az*ACC_SENSITIVITY;
    float f_gx = gx*GYRO_SENSITIVITY - gyro_bias_x;
    float f_gy = gy*GYRO_SENSITIVITY - gyro_bias_y;
    float f_gz = gz*GYRO_SENSITIVITY - gyro_bias_z;

    int64_t now = esp_timer_get_time();
    float dt = (last_time_us==0)?0.01f:(now-last_time_us)/1e6f;
    if(dt<=0 || dt>0.1f) dt=0.01f;
    last_time_us=now;

    g_roll += f_gx*dt;
    g_pitch += f_gy*dt;
    g_yaw += f_gz*dt;

    float roll_acc = atan2f(f_ay,f_az)*RAD2DEG;
    float pitch_acc = atan2f(-f_ax,sqrtf(f_ay*f_ay+f_az*f_az))*RAD2DEG;

    const float alpha=0.98f;
    g_roll=alpha*g_roll+(1-alpha)*roll_acc;
    g_pitch=alpha*g_pitch+(1-alpha)*pitch_acc;

    att->roll=g_roll;
    att->pitch=g_pitch;
    att->yaw=g_yaw;
}

// ---------------------- 陀螺标定 ----------------------
static void ICM42688_CalibrateGyro(void){
    const int N=500;
    double sx=0,sy=0,sz=0;
    for(int i=0;i<N;i++){
        int16_t ax,ay,az,gx,gy,gz;
        ICM42688_getacc(&ax,&ay,&az);
        ICM42688_getgyro(&gx,&gy,&gz);
        sx+=gx*GYRO_SENSITIVITY;
        sy+=gy*GYRO_SENSITIVITY;
        sz+=gz*GYRO_SENSITIVITY;
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    gyro_bias_x=(float)(sx/N);
    gyro_bias_y=(float)(sy/N);
    gyro_bias_z=(float)(sz/N);
    gyro_calibrated=1;
    ESP_LOGI(TAG,"Gyro bias calibrated: %.3f, %.3f, %.3f",gyro_bias_x,gyro_bias_y,gyro_bias_z);
}
