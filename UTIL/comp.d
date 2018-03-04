import std.stdio, std.string, std.stream;
import util;

int main(char[][] args) {
	if (args.length < 3) { writefln("divicomp [c|u] <input> <output> ..."); return 0; }
	if (args.length % 2) throw(new Exception("Each input must have an output"));
	
	void function(char[], char[]) func;
	
	switch (args[1]) {
		case "c": func = &compress_file  ; break;
		case "u": func = &uncompress_file; break;
		default: throw(new Exception(std.string.format("Unknown command '%s'", args[1]))); return 0;
	}

	for (int n = 2; n < args.length - 1; n += 2) {
		char[] name_in = args[n + 0], name_out = args[n + 1];
		if (name_in == name_out) throw(new Exception("Input must be different from output"));
		writefln("%s -> %s", name_in, name_out);
		func(name_in, name_out);
	}
	
	return 0;
}