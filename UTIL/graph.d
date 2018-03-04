import std.stdio, std.string, std.file, std.stream, std.conv;
import util;

void getTranslated(char[] name, out int count, out int total) {
	Stream s = new File("../ACME/SRC/" ~ name ~ ".txt");
	total = count = 0;
	while (!s.eof) {
		char[] line = std.string.strip(s.readLine);
		if (line.length < 10) continue;
		if (line[0..10] != "## POINTER") continue;
		total++;
		if (std.string.find(line, "t:1") != -1) count++;
	}
	s.close();
}

void analyze_script() {
	char[] path;
	
	char[] normalize(char[] name) {
		//writefln(name);
		for (int n = 0; n < name.length; n++) {
			if (name[n] == '.') {
				name = name[0..n];
				break;
			}
		}
		return toupper(name);
	}
	
	bool[char[]] ends;
	bool[char[]] list;
	bool dont_check;
	
	//dont_check = true;
	
	if (dont_check || !exists("graph/links.txt")) {
		Stream slinks = new File("graph/links.txt", FileMode.OutNew);
		listdir(path = "../SG", delegate bool(char[] f) {
			char[] rf = path ~ "/" ~ f;
			if (f.length >= 3 && f[f.length-3..f.length] == ".AB") {
				Script.decompile(new File(rf), null, null, true);
				//writef("\t\"%s\"", f); foreach (cf; Script.ana_links) writef(" -> \"%s\"", cf); writefln(";");

				char[] n1 = normalize(f);
				
				if (Script.ana_end) {
					ends[n1] = true;
					//fprintf(stderr, "END\n");
				}
					
				list[n1] = true;

				foreach (cf; Script.ana_links) {
					char[] n2 = normalize(cf);
					if (!n2.length) {
						fprintf(stderr, "Invalid reference '%s' in '%s'\n", std.string.toStringz(n2), std.string.toStringz(n1));
						continue;
					}
					slinks.writefln("\"%s\" -> \"%s\"", n1, n2);
				}
			}
			
			return true;
		});
		slinks.close();		
		
		slinks = new File("graph/list.txt", FileMode.OutNew);
		foreach (v; list.keys) {
			int type = 0;
			if (v in ends) type = 1;
			if (v == "AASTART") type = 2;
			int count, total;
			
			getTranslated(v, count, total);
			
			slinks.writefln("%d,%d,%d,%s", type, count, total, v);
		}
		slinks.close();
	}
	
	//return;

	writefln("digraph SCRIPT {");
		writefln("concentrate=true");
		writefln("size=\"12,90\";");
		writefln("ratio=compress;");
		//writefln("nodesep=.05;");
		//writefln("rankdir=LR;");
		
		writefln("node [style=filled,arrowsize=0.3,fontsize=12];");
		writefln("%s", cast(char[])read("graph/links.txt"));
		Stream s = new File("graph/list.txt");
		while (!s.eof) {
			char[][] args = split(s.readLine, ",");
			int type = toInt(args[0]);
			int count = toInt(args[1]);
			int total = toInt(args[2]);
			char[] name = args[3];
			char[] params;
			
			int n = (count * 0xFF) / total;
			int o = (n < 127) ? 0xFF : 0x00;
			
			int n_r = n, n_g = n, n_b = n;
			int o_r = o, o_g = o, o_b = o;

			n = ((n * 200) / 256) + 28;
			
			if (count == total) {
				o_r = 0x00;
				o_g = 0x5F;
				o_b = 0x00;
				n_r = 0x80;
				n_g = 0xF0;
				n_b = 0x80;
			}

			switch (type) {
				case 0: params = std.string.format("[shape=ellipse,color=\"#%02X%02X%02X\",fontcolor=\"#%02X%02X%02X\"]", n_r, n_g, n_b, o_r, o_g, o_b); break;
				case 1: params = std.string.format("[shape=parallelogram,color=\"#%02X%02X%02X\",fontcolor=\"#ff0000\"]", n_r, n_g, n_b); break;
				case 2: params = std.string.format("[shape=house,color=\"#%02X%02X%02X\",fontcolor=\"#0000ff\"]", n_r, n_g, n_b); break;
			}
			
			if (params.length) writefln("\"%s\" %s;", name, params);			
		}
	writefln("}");
}

int main(char[][] args) {
	try { mkdir("graph"); } catch { }
	analyze_script();
		
	return 0;
}