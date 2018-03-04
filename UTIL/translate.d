import std.stdio, std.string, std.file, std.stream;
import util;

void decompile_script() {
	char[] path;
	try { mkdir("ACME"); } catch { }
	
	//unpack("SG");
	
	listdir(path = "SG", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		if (f.length >= 3 && f[f.length-3..f.length] == ".AB") {
			char[] base = f[0..f.length - 3];
			writefln("%s", rf);
			
			Script.decompile(
				new File(rf),
				new File("ACME/" ~ base ~ ".ABS", FileMode.OutNew),
				new MemoryStream(),
				true
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

	Stream s = new File("LANG/SPANISH.DL1", FileMode.OutNew);
	s.writeString("DL1.0\x1A\0\0");
	s.write(cast(ushort)0);
	s.write(cast(uint)0);
	s.write(cast(ushort)0);
	
	listdir(path = "PAK", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		
		//bool comp = false;
		bool comp = true; // compress all
		bool apply = false;
		
		if (f.length >= 3 && f[f.length-3..f.length] == ".AB" ) apply = true;
		if (f.length >= 4 && f[f.length-4..f.length] == ".BMP") { apply = true; comp = true; }
		
		if (apply) {
			ubyte[] data = cast(ubyte[])std.file.read(rf);
			int st = s.position;
			if (comp) compress_stream(data, s); else s.write(data);
			entries ~= Entry(f, s.position - st);
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

void compile_script() {
	char[] path;
	
	try { mkdir("PAK"); } catch { }
	
	listdir(path = "ACME", delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		
		if (f.length >= 4 && f[f.length-4..f.length] == ".ABS") {
			char[] base = f[0..f.length - 4];
			writefln("%s...", rf);
			Script.compile(
				new File("PAK/" ~ base ~ ".AB", FileMode.OutNew),
				new File("ACME/" ~ base ~ ".ABS"),
				//new File("ACME/SRC/" ~ base ~ ".txt")
				new File("ACME/dividead/" ~ base ~ ".bin")
			);
		}
		
		return true;
	});
}

int main(char[][] args) {
	char[] path;
	bool done = false;

	//writefln("unpack_sg");
	//unpack("SG");
	//writefln("decompile_script");
	//decompile_script();
	writefln("compile_script");
	compile_script();
	
	foreach (s; ["M_A1", "M_A2", "M_B1", "M_B2"]) {
		try { copy("RES/MAP/" ~ s ~ ".BMP", "PAK/" ~ s ~ ".BMP"); } catch (Exception e) { writefln(e); }
	}
	foreach (s; ["OMAKE_5A", "OMAKE_6A", "OMAKE_8A", "OMAKE_9A", "OMAKE_0B", "OMAKE_2A", "OMAKE_7"]) {
		try { copy("RES/OMAKE/" ~ s ~ ".BMP", "PAK/" ~ s ~ ".BMP"); } catch (Exception e) { writefln(e); }
	}
	
	writefln("repack");
	repack();
	
	// REMOVE
	//if (true) {
	if (false) {
		listdir(path = "PAK", delegate bool(char[] f) {
			char[] rf = path ~ "/" ~ f;
			std.file.remove(rf);
			return true;
		});
		try { rmdir("PAK"); } catch { }
	}

	return 0;
}