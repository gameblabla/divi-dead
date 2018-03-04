import std.stdio, std.string, std.file, std.stream;
import util;

void analyze_script() {
	char[] path;
	
	listdir(path = "../SG", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		if (f.length >= 3 && f[f.length-3..f.length] == ".AB") {
			Script.decompile(new File(rf), null, null, true);
		}
		
		return true;
	});
	
	Script.showAnalytics();
}

void decompile_script() {
	char[] path;
	try { mkdir("../ABS"); } catch { }
	try { mkdir("../ABS/FULL"); } catch { }
	listdir(path = "../SG", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		if (f.length >= 3 && f[f.length-3..f.length] == ".AB") {
			char[] base = f[0..f.length - 3];
			writefln("%s", rf);
			
			try { mkdir("../ABS/SRC"); } catch { }
			
			Script.decompile(
				new File(rf),
				new File("../ABS/" ~ base ~ ".ABS", FileMode.OutNew),
				new File("../ABS/SRC/" ~ base ~ ".txt", FileMode.OutNew),
				true
			);
			
			Script.decompile(
				new File(rf),
				new File("../ABS/FULL/" ~ base ~ ".ABS", FileMode.OutNew)
			);
		}
		
		return true;
	});
}

void repack() {
	char[] path;
	
	struct Entry {
		char[] name;
		int size;
	}
	
	Entry[] entries;

	Stream s = new File("../LANG/SPANISH.DL1", FileMode.OutNew);
	s.writeString("DL1.0\x1A\0\0");
	s.write(cast(ushort)0);
	s.write(cast(uint)0);
	s.write(cast(ushort)0);
	
	listdir(path = "../PAK", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		if (f.length >= 3 && f[f.length-3..f.length] == ".AB") {
			ubyte[] data = cast(ubyte[])std.file.read(rf);
			s.write(data);
			entries ~= Entry(f, data.length);
		}
		
		return true;
	});

	while (s.position % 0x10) s.write(cast(ubyte)0);
	
	int start = s.position;
	s.position = 8;
	s.write(cast(ushort)entries.length);
	s.write(cast(uint)start);
	s.position = start;
	foreach (e; entries) {
		ubyte[0xC] file;
		file[0..e.name.length] = cast(ubyte[])e.name;
		s.write(file);
		s.write(cast(uint)e.size);
	}
}

void compile_script(bool use_full = false) {
	char[] path;
	
	if (use_full) {
		listdir(path = "../ABS/FULL", delegate bool(char[] f) {
			char[] rf = path ~ "/" ~ f;
			
			if (f.length >= 4 && f[f.length-4..f.length] == ".ABS") {
				char[] base = f[0..f.length - 4];
				writefln("%s", rf);
				Script.compile(
					new File("../PAK/" ~ base ~ ".AB", FileMode.OutNew),
					new File("../ABS/FULL/" ~ base ~ ".ABS")
				);
			}
			
			return true;
		});
	} else {
		listdir(path = "../ABS", delegate bool(char[] f) {
			char[] rf = path ~ "/" ~ f;
			
			if (f.length >= 4 && f[f.length-4..f.length] == ".ABS") {
				char[] base = f[0..f.length - 4];
				writefln("%s", rf);
				Script.compile(
					new File("../PAK/" ~ base ~ ".AB", FileMode.OutNew),
					new File("../ABS/" ~ base ~ ".ABS"),
					new File("../ABS/SRC/" ~ base ~ ".txt")
					//new File("../ABS/dividead/" ~ base ~ ".txt")
				);
			}
			
			return true;
		});
	}
}

int main(char[][] args) {
	bool done = false;

	/*Script.decompile(
		new File("../SG/F10_25.AB"),
		new File("TEST.ABS", FileMode.OutNew)
	);*/

	/*Script.compile(
		new File("../ABS/FULL/F15_12.ABS"),
		new File("TEST.AB", FileMode.OutNew)
	);*/

	//Script.decompile(new File("../SG/F27_41.AB"));
	//Script.decompile(new File("../sg/AASTART.AB"));

	if (args.length >= 2) {
		foreach (cmd; args[1..args.length]) {
			done = true;
			switch (cmd) {
				case "a": analyze_script(); break;
				case "d": decompile_script(); break;
				case "c": compile_script(false); repack(); break;
				case "cf": compile_script(true); repack(); break;
				default:
					writefln("Unknown option '%s'", cmd);
					return -1;
				break;
			}
		}
	}
	
	if (!done) {
		writefln("script (d|c|cf|a)");
		writefln("d  - decompile script (SG -> ABS)");
		writefln("c  - compile script (ABS -> SG)");
		writefln("cf - compile full_script (ABS/FULL -> SG)");
		writefln("a  - analyze script");
		return -1;
	}
	
	return 0;
}