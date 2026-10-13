"""Compile actual snake.c for ARM and run its tests in Unicorn (pip install unicorn)."""
from pathlib import Path
import subprocess, sys
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_R0

root=Path(__file__).resolve().parents[1]
ide=Path(sys.argv[1]) if len(sys.argv)>1 else Path('D:/CubeIDE/STM32CubeIDE_1.19.0/STM32CubeIDE')
gcc=next((ide/'plugins').rglob('arm-none-eabi-gcc.exe'))
out=root/'Build/tests'; out.mkdir(parents=True,exist_ok=True)
subprocess.run([str(gcc),'-mcpu=cortex-m4','-mthumb','-O1','-g','-Wall','-Wextra','-Werror',
    '-nostartfiles','--specs=nosys.specs','-Wl,-Ttext=0x10000,-e,test_all',
    '-I'+str(root/'Core/Inc'),str(root/'Core/Src/snake.c'),str(root/'tests/test_snake.c'),
    '-o',str(out/'test.elf')],check=True)
subprocess.run([str(gcc.with_name('arm-none-eabi-objcopy.exe')),'-O','binary',str(out/'test.elf'),str(out/'test.bin')],check=True)
symbols=subprocess.check_output([str(gcc.with_name('arm-none-eabi-nm.exe')),str(out/'test.elf')],text=True)
entry=int(next(line.split()[0] for line in symbols.splitlines() if line.endswith(' test_all')),16)
uc=Uc(UC_ARCH_ARM,UC_MODE_THUMB)
uc.mem_map(0x10000,0x100000)
uc.mem_write(0x10000,(out/'test.bin').read_bytes())
uc.mem_map(0x20000000,0x10000)
uc.reg_write(UC_ARM_REG_SP,0x2000fff0)
uc.reg_write(UC_ARM_REG_LR,0x100001)
uc.emu_start(entry|1,0x100000,count=20000000)
from unicorn.arm_const import UC_ARM_REG_PC
assert uc.reg_read(UC_ARM_REG_PC)==0x100000, 'Instruction limit reached'
result=uc.reg_read(UC_ARM_REG_R0)
assert result==0, f'test_snake.c failed at line {result}'
print('PASS: movement, reverse/rapid turns, pause, restart, growth, food, walls, tail, self-collision, win, 1000 seeds')
