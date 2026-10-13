param([string]$CubeIDE = 'D:\CubeIDE\STM32CubeIDE_1.19.0\STM32CubeIDE')
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $gcc = Get-ChildItem -LiteralPath "$CubeIDE\plugins" -Filter arm-none-eabi-gcc.exe -Recurse | Select-Object -First 1
    if (!$gcc) { throw 'ARM compiler not found. Pass -CubeIDE with your STM32CubeIDE directory.' }
    $bin = $gcc.DirectoryName
    $out = Join-Path $PSScriptRoot 'Build'
    New-Item -ItemType Directory -Force -Path $out | Out-Null
    $inc = @('Core/Inc','Drivers/STM32F4xx_HAL_Driver/Inc','Drivers/CMSIS/Include',
        'Drivers/CMSIS/Device/ST/STM32F4xx/Include','Middlewares/Third_Party/FreeRTOS/Source/include',
        'Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS','Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F')
    $cpu = @('-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=hard')
    $flags = $cpu + @('-g3','-Og','-ffunction-sections','-fdata-sections','-Wall','-Wextra',
        '-DSTM32F427xx','-DUSE_HAL_DRIVER','--specs=nano.specs') + @($inc | ForEach-Object { "-I$_" })
    $sources = Get-ChildItem Core,Drivers,Middlewares,startup -Recurse -File | Where-Object { $_.Extension -in '.c','.s' }
    $objects = @()
    foreach ($source in $sources) {
        $relative = $source.FullName.Substring($PSScriptRoot.Length + 1)
        $obj = Join-Path $out ($relative.Replace('\','_').Replace('/','_') + '.o')
        $extra = @()
        if ($relative.StartsWith('Middlewares')) { $extra += '-Wno-unused-parameter' }
        & $gcc.FullName @flags @extra -c $source.FullName -o $obj
        if ($LASTEXITCODE) { throw "Compilation failed: $relative" }
        $objects += $obj
    }
    & $gcc.FullName @cpu --specs=nano.specs --specs=nosys.specs -T STM32F427VITx_FLASH.ld '-Wl,--gc-sections,-Map=Build/svaga.map' @objects -o Build/svaga.elf
    if ($LASTEXITCODE) { throw 'Link failed' }
    & "$bin/arm-none-eabi-objcopy.exe" -O ihex Build/svaga.elf Build/svaga.hex
    if ($LASTEXITCODE) { throw 'HEX export failed' }
    & "$bin/arm-none-eabi-objcopy.exe" -O binary Build/svaga.elf Build/svaga.bin
    if ($LASTEXITCODE) { throw 'BIN export failed' }
    & "$bin/arm-none-eabi-size.exe" Build/svaga.elf
    if ($LASTEXITCODE) { throw 'Size check failed' }
} finally { Pop-Location }
