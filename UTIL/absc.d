import std.stdio, std.string, std.file, std.stream;
import util;

void compile_script(char[] name) {
	char[][] args = std.string.split(name, ".");
	name = args[0];
	
	Script.compile(new File(name ~ ".AB", FileMode.OutNew), new File(name ~ ".ABS"));
}

int main(char[][] args) {
	if (args.length < 2) {
		writefln("absc file.abs");
		return -1;
	}
	compile_script(args[1]);
	return 0;
}