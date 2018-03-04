import std.stdio, std.stream, std.string, std.path, std.file;
import util;

void extract(char[] path) {
	writefln("%s", path);
	struct Entry {
		char[] name;
		Stream slice;
	}
	
	Entry[] es;
	Stream s = new File(path ~ ".DL1", FileMode.In);
	ubyte[8] sign;
	ushort count;
	uint addr;
	s.read(sign);
	if (sign != cast(ubyte[])"DL1.0\x1A\0\0") throw(new Exception("Not a DL1.0 file"));
	s.read(count);
	s.read(addr);
	s.seekSet = addr;
	
	int current = 0x10;
	for (int n = 0; n < count; n++) {
		char[] fname;
		ubyte[0xC] file;
		uint length;
		s.read(file);
		s.read(length);
		foreach (c; file) { if (c == 0) break; fname ~= c; }
		
		uint back = current;
		current += length;
		
		es ~= Entry(fname, new SliceStream(s, back, current));
	}
	
	try { mkdir(path); } catch { }
	
	foreach (e; es) {
		char[] fname = path ~ "/" ~ e.name;
		char[][] namep = split(e.name, ".");
		char[] ext = (namep.length < 2) ? "" : namep[1];
		
		printf("  %s\t\t\r", std.string.toStringz(e.name));
		
		if (!std.file.exists(fname)) {
			char[] magic;
			magic = e.slice.readString(2);
			e.slice.position = 0;
			
			if (magic == "LZ") {
				write(fname, uncompress_stream(e.slice));
			} else {
				Stream ss = new File(fname, FileMode.OutNew);
				ss.copyFrom(e.slice);
				ss.close();
			}
		}
	}
	writefln();
}

int main(char[][] args) {
	extract(r"..\SG");
	extract(r"..\WV");

	return 0;
}