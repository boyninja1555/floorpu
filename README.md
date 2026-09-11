<div align="center">
	<h1>FloorPU</h1>
	<p>The <em>Floor Processing Unit</em>, at its finest!</p>
</div>

---

One time I wanted to make an 8-bit emulated retro game CPU, but I didn't realize *8-bit* referred to the register size! So, because of my experience with the **GBA**, I MADE IT 32-BIT WAHOOOOOOO!!

*ahem* **HOWEVER**, my memory was byte-addressable and I had no stack. You can see the issue, right? I became very familiar with my `LOAD32` and `LOAD16`! Oh and it was big-endian and that's a big no-no for some reason.

**YAY NEW PLAN TIME!!**

Little-endian, *actually* 8-bit, and no graphics currently! Just write a program and see the register values per-cycle.

Am I proud? Yes! But only because I wrote the assembler using the more common *lex/parse* pattern instead of vibe coding a shaky space-splitter Python script (boooooo). The actual emulator is basic stuff: consume opcode and consume operands.

> **For context** I did not vibe code this new one, nor most of the old project!

Oh and I'm not writing ISA documentation this time. If you can't read the emulator code yourself, are you really fit to make a game in raw Assembly? **No!**

**How to do shit??**

You're gonna need CMake, an ISO C 17 compiler like `gcc`, and Linux! ISO C 17 is a must-have, it just feels different than 11 idrk.

To run a ROMfile (linux only cus why not :P):

```bash
./build.sh Debug
build/FloorPU programs/addfunc.rom
```

Remember to look at the register values in your terminal! `r0` for this example should contain the result of `1+1`.

To assemble Assembly source (say that 31415926^π times):

```bash
cd assembler
./build.sh Debug
build/FpuASM ../programs/addfunc.asm ../programs/addfunc.rom
```
