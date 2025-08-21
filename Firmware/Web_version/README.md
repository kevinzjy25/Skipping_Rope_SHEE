{
  "configurations": [
    {
      "name": "ESP-IDF",
      "compilerPath": "${config:idf.toolsPathWin}\\tools\\xtensa-esp-elf\\esp-14.2.0_20241119\\xtensa-esp-elf\\bin\\xtensa-esp32-elf-gcc.exe",
      "compileCommands": "${config:idf.buildPath}/compile_commands.json",
      "includePath": [
        "${config:idf.espIdfPath}/components/**",
        "${config:idf.espIdfPathWin}/components/**",
        "${workspaceFolder}/**"
      ],
      "browse": {
        "path": [
          "${config:idf.espIdfPath}/components",
          "${config:idf.espIdfPathWin}/components",
          "${workspaceFolder}"
        ],
        "limitSymbolsToIncludedHeaders": true
      }
    }
  ],
  "version": 4
}
"clangd.arguments": [
    "--background-index",
    "--query-driver=e:\\Program-Files\\ESP_idf_Tool\\tools\\xtensa-esp-elf\\esp-14.2.0_20241119\\xtensa-esp-elf\\bin\\xtensa-esp32-elf-gcc.exe",
    "--compile-commands-dir=${workspaceFolder}\\build"
]
"clangd.path": "e:\\Program-Files\\ESP_idf_Tool\\tools\\esp-clang\\esp-18.1.2_20240912\\esp-clang\\bin\\clangd.exe",