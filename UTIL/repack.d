import std.stdio, std.stream, std.string, std.path, std.file;
import util;

void repack(char[] ofile, char[] path) {
	struct Entry {
		char[] name;
		int size;
	}
	
	Entry[] entries;

	Stream s = new File(ofile, FileMode.OutNew);
	s.writeString("DL1.0\x1A\0\0");
	s.write(cast(ushort)0);
	s.write(cast(uint)0);
	s.write(cast(ushort)0);
	
	listdir(path, delegate bool(char[] f) {
		char[] rf = path ~ "/" ~ f;
		
		bool comp = false;
		//bool comp = true; // compress all
		bool apply = false;
		
		//if (f.length >= 3 && f[f.length-3..f.length] == ".AB" ) apply = true;
		//if (f.length >= 4 && f[f.length-4..f.length] == ".BMP") { apply = true; comp = true; }
		if (f.length >= 4 && f[f.length-4..f.length] == ".BMP") comp = true;
		
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

int main(char[][] args) {
	if (args.length < 3) {
		writefln("pack file.dl1 directory");
		return - 1;
	}
	
	repack(args[1], args[2]);
	
	return 0;
}