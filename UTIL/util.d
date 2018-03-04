module util;

import std.stdio, std.file, std.stream, std.string;

//version = profile;

extern (C) int dv_compress(void *, int, void *, int *);

ubyte[] compress(ubyte[] input) {
	ubyte[] output = new ubyte[(input.length * 9 + 1) / 8];
	int len = output.length;
	dv_compress(input.ptr, input.length, output.ptr, &len);
	output.length = len;
	return output;
}

void compress_stream(ubyte[] input, Stream so) {
	ubyte[] output = compress(input);
	so.writeString("LZ");
	so.write(cast(uint)output.length);
	so.write(cast(uint)input.length);
	so.write(output);
}

void compress_file(char[] name_in, char[] name_out) {
	Stream sout = new File(name_out, FileMode.OutNew);
	compress_stream(cast(ubyte[])read(name_in), sout);
	sout.close();
}

void uncompress(ubyte[] input, ubyte[] output) {
	ubyte[0x1000] ring; ushort rinp = 0xFEE;
	ubyte* ptr = input.ptr, end = ptr + input.length, outp = output.ptr;
	
	final ubyte  get() { return *ptr++; }
	final ushort get2() { scope (exit) ptr += 2; return *cast(short *)ptr; }
	final void put(ubyte v) { *outp = v; outp++; ring[rinp] = v; rinp++; rinp &= 0xFFF; }
	
	while (ptr < end) {
		uint code = *ptr | 0x100;
		version (profile) writefln("CODE: %02X", code & 0xFF);
		for (ptr++; code != 1; code >>= 1) {
			if (code & 1) {
				version (profile) writefln("BYTE: %02X", *ptr);
				put(get);
			} else {
				if (ptr >= end) break;
				ushort d = get2;
				ushort p = (d & 0xFF) | ((d >> 4) & 0xF00);
				ushort s = ((d >> 8) & 0xF) + 3;
				version (profile) writefln("UNCOMP: %d, %d", p, s);
				while (s--) { version (profile) writefln("  BYTE: %02X", ring[p]); put(ring[p++]); p &= 0xFFF; }
			}
		}
	}
}

ubyte[] uncompress_stream(Stream si) {
	ubyte[2] hd;
	uint size_i, size_o;
	si.read(hd);
	if (hd != cast(ubyte[])"LZ") throw(new Exception("Not a compressed stream"));
	si.read(size_i);
	si.read(size_o);
	
	ubyte[] buf_i = new ubyte[size_i];
	ubyte[] buf_o = new ubyte[size_o];
	
	si.read(buf_i);
	
	uncompress(buf_i, buf_o);
	
	delete buf_i;
	
	return buf_o;
}

void uncompress_file(char[] name_in, char[] name_out) {
	ubyte[] buf_in = cast(ubyte[])read(name_in);
	if (buf_in[0..2] != cast(ubyte[])"LZ") throw(new Exception("Not a compressed file"));
	uint size_in  = *cast(uint*)(buf_in.ptr + 2 + 0);
	uint size_out = *cast(uint*)(buf_in.ptr + 2 + 4);
	ubyte[] buf_out = new ubyte[size_out];
	uncompress(buf_in[10..10 + size_in], buf_out);
	write(name_out, buf_out);
}

final int min(int a, int b) { return (a < b) ? a : b; }

void compress(ubyte[] input, inout ubyte[] output, int method = 0) {
	switch (method) {
		// Fastest
		case 0:
			int op = 0;
			int ip = 0;
			while (true) {
				int count = min(input.length - ip, 8);
				output[op++] = (1 << count) - 1;
				output[op..op + count] = input[ip..ip + count];
				op += count; ip += count;
				if (count < 8) break;
			}
			output.length = op;
		break;
		// Good
		default: case 1:
			throw(new Exception("Not implemented"));
		break;
	}
}

/*
void compress_file(char[] name_in, char[] name_out) {
	ubyte[] buf_u = cast(ubyte[])read(name_in);
	ubyte[] buf_c = new ubyte[((buf_u.length * 9) / 8) + 0x10];
	
	compress(buf_u, buf_c);

	Stream s_out = new File(name_out, FileMode.OutNew);
	s_out.writeString("LZ");
	s_out.write(cast(uint)(buf_c.length));
	s_out.write(cast(uint)(buf_u.length));
	s_out.write(buf_c);
	s_out.close();
}
*/

struct OPCODE {
	ushort id;
	char[] params;
	char[] name;
	int count;
}

OPCODE[char[]] opcodes_name;

OPCODE[] opcodes = [
	OPCODE(0x0000, "T"     , "TEXT"),             // text)
	OPCODE(0x0001, "PT"    , "OPTION"),           // jump_addr, text
	OPCODE(0x0002, "P"     , "JUMP"),             // jump_addr
	OPCODE(0x0003, "222"   , "SET_RANGE"),        // flag number1 0-999, flag number2 0-999, flag value @@ SET_RANGE 0, 999, 0 (clean all flags)
	OPCODE(0x0004, "2c2"   , "SET"),              // flag number1 0-999, op '=', value (ops: '=', '+', '-')
	OPCODE(0x0005, ""      , "?"),                // not using
	OPCODE(0x0006, ""      , "OPTION_RESET"),     //
	OPCODE(0x0007, ""      , "OPTION_SHOW"),      //
	OPCODE(0x0008, ""      , "-"),                //
	OPCODE(0x0009, ""      , "-"),                //
	OPCODE(0x000A, ""      , "OPTION_RESHOW"),    // Recall OPTION_SHOW mantaining last cursor
	OPCODE(0x000B, ""      , "-"),                //
	OPCODE(0x000C, ""      , "-"),                //
	OPCODE(0x000D, ""      , "?"),                // not using
	OPCODE(0x000E, ""      , "?"),                // not using
	OPCODE(0x000F, ""      , "?"),                // not using
	OPCODE(0x0010, "2c2P"  , "JUMP_IF_NOT"),      // flag number1 0-999, op '=', value, jump_addr (ops: '=', '}', '{')
	OPCODE(0x0011, "2"     , "WAIT_MS"),          // millisec
	OPCODE(0x0012, ""      , "?"),                // not using
	OPCODE(0x0013, ""      , "?"),                // not using
	OPCODE(0x0014, "2"     , "REPAINT"),          // repaint screen using an effect
	OPCODE(0x0015, ""      , "-"),                //
	OPCODE(0x0016, "S"     , "IMAGE_OVERLAY"),    //
	OPCODE(0x0017, ""      , "?"),                // not using
	OPCODE(0x0018, "S"     , "SCRIPT"),           // script without .ab
	OPCODE(0x0019, ""      , "GAME_END"),         //
	OPCODE(0x001A, ""      , "-"),                //
	OPCODE(0x001B, ""      , "-"),                //
	OPCODE(0x001C, ""      , "-"),                //
	OPCODE(0x001D, ""      , "-"),                //
	OPCODE(0x001E, ""      , "FADE_OUT"),         //
	OPCODE(0x001F, ""      , "FADE_OUT_WHITE"),   //
	OPCODE(0x0020, ""      , "-"),                //
	OPCODE(0x0021, ""      , "-"),                //
	OPCODE(0x0022, ""      , "-"),                //
	OPCODE(0x0023, ""      , "-"),                //
	OPCODE(0x0024, ""      , "-"),                //
	OPCODE(0x0025, ""      , "?"),                // not using
	OPCODE(0x0026, "S"     , "MUSIC"),            // midi without .mid
	OPCODE(0x0027, ""      , "-"),                //
	OPCODE(0x0028, ""      , "MUSIC_STOP"),       //
	OPCODE(0x0029, ""      , "?"),                // not using
	OPCODE(0x002A, ""      , "-"),                //
	OPCODE(0x002B, "S"     , "VOICE"),            // wav without .wav
	OPCODE(0x002C, ""      , "-"),                //
	OPCODE(0x002D, ""      , "-"),                //
	OPCODE(0x002E, ""      , "-"),                //
	OPCODE(0x002F, ""      , "-"),                //
	OPCODE(0x0030, "2222"  , "CLIP"),             // x1, y1, x2, y2
	OPCODE(0x0031, ""      , "?"),                // not using
	OPCODE(0x0032, ""      , "?"),                // not using
	OPCODE(0x0033, ""      , "?"),                // not using
	OPCODE(0x0034, ""      , "-"),                //
	OPCODE(0x0035, "S"     , "SOUND"),            // wav without .wav
	OPCODE(0x0036, ""      , "SOUND_STOP"),       // pauses a possible sound playing
	OPCODE(0x0037, "SS"    , "MAP_IMAGES"),       // image1 (without focus), image2 (with focus)
	OPCODE(0x0038, ""      , "MAP_OPTION_RESET"), //
	OPCODE(0x0039, ""      , "-"),                //
	OPCODE(0x003A, ""      , "-"),                //
	OPCODE(0x003B, ""      , "-"),                //
	OPCODE(0x003C, ""      , "-"),                //
	OPCODE(0x003D, ""      , "-"),                //
	OPCODE(0x003E, ""      , "-"),                //
	OPCODE(0x003F, ""      , "-"),                //
	OPCODE(0x0040, "P2222" , "MAP_OPTION"),       // jump_addr, x1, y1, x2, y2
	OPCODE(0x0041, ""      , "MAP_OPTION_SHOW"),  //
	OPCODE(0x0042, ""      , "?"),                // not using
	OPCODE(0x0043, ""      , "?"),                // not using
	OPCODE(0x0044, ""      , "?"),                // not using
	OPCODE(0x0045, ""      , "?"),                // not using
	OPCODE(0x0046, "S"     , "BACKGROUND1"),      // image without .bmp
	OPCODE(0x0047, "s"     , "BACKGROUND2"),      // image without .bmp
	OPCODE(0x0048, ""      , "?"),                // not using
	OPCODE(0x0049, ""      , "?"),                // not using
	OPCODE(0x004A, "2"     , "REPAINT_IN"),       // type
	OPCODE(0x004B, "S"     , "CHARA"),            // image without .bmp applied (alpha) _0
	OPCODE(0x004C, "SS"    , "CHARA2"),           // two characters
	OPCODE(0x004D, ""      , "ANIMATION"),        //
	OPCODE(0x004E, ""      , "SCROLL_DOWN"),      //
	OPCODE(0x004F, ""      , "SCROLL_UP"),        //
	OPCODE(0x0050, "T"     , "SAVE_TITLE"),       // 11 characters max
];

static this() {
	foreach (op; opcodes) opcodes_name[op.name] = op;
}

class Script {
	static char[] trans;
	static char[] trans_r;

	static this() {
		//char[] s1 = "@", s2 = "\"";
		char[] s1 = "", s2 = "";
			
		trans   = maketrans(s1, s2);
		trans_r = maketrans(s2, s1);
	}

	static char[] decodeString(char[] s) {
		s = replace(replace(s, "\\\\W", ""), "\\", "");
		
		return translate(s, trans, "");
	}

	static char[] encodeString(char[] s) {
		return translate(s, trans_r, "");
	}
	
	static char[] addslashes(char[] s) {
		char[] r;
		foreach (c; s) {
			switch (c) {
				case '"': case '\\': r ~= "\\" ~ c; break;
				default: r ~= c;
			}
		}
		return r;
	}
	
	static void compile(Stream s_out, Stream s_script, Stream s_table = null) {
		struct PATCH {
			char[] label;
			int addr;
		}
		char[][] strings;
		uint[char[]] labels;
		PATCH[] patches;
	
		final static char[][] tokenize(char[] line) {
			char[][] r; char[] cc;
			
			line ~= " ";
			for (int n = 0; n < line.length; n++) {
				char c = line[n];
				switch (c) {
					// separator
					case ' ': case '\n': case '\r': case '\t': case ',':
						if (!cc.length) break;
						r ~= cc.dup;
						cc.length = 0;
					break;
					// string
					case '"':
						cc ~= "#";
						for (n++; n < line.length; n++) {
							c = line[n];
							if (c == '"') break;
							if (c == '\\') c = line[++n];
							cc ~= c;
						}
					break;
					// char
					case '\'':
						cc ~= line[++n];
						n++;
					break;
					// character
					default: cc ~= c; break;
				}
			}
			
			return r;
		}
		
		final static uint toInt(char[] s, bool ignore = true) {
			uint r;
			int base = 10;
			
			if (s.length >= 2 && s[0..2] == "0x") {
				base = 16;
				s = s[2..s.length];
			} else if (s.length >= 2 && s[0..2] == "0b") {
				base = 2;
				s = s[2..s.length];
			} else if (s.length >= 1 && s[0] == '0') {
				base = 8;
				s = s[1..s.length];
			}

			try {
				foreach (c; s) {
					int v;
					if (c >= '0' && c <= '9') v = c - '0';
					else if (c >= 'A' && c <= 'Z') v = c - 'A' + 10;
					else if (c >= 'a' && c <= 'z') v = c - 'a' + 10;
					else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
					else throw(new Exception(std.string.format("Invalid numeric character (%02X)", cast(ubyte)c)));
					if (v >= base) throw(new Exception(std.string.format("Invalid digit for base (%d)", v)));
					r *= base;
					r += v;
				}
			} catch (Exception e) {
				if (!ignore) throw(e);
			}
			
			return r;
		}
	
		void read_strings() {
			alias s_table s;
			if (!s_table) return;
			int pos = 0;
			ubyte[4] header;
			s_table.read(header);
			if (header == cast(ubyte[])"ACM2") {
				char[][] cols;
				char[] col;
				ushort ver, ncols;
				s.read(ver); s.read(ncols);
				while (ncols--) { s.read(col); cols ~= col; }
				while (!s.eof) {
					char[][char[]] row;
					foreach (ccol; cols) {
						char[] text;
						s.read(text);
						row[ccol] = text;
					}
					
					//strings[toInt(row["key"])] = row["t2"];
					strings ~= row["t2"];
				}
				//count: 54
			} else {
				s_table.position = 0;
				while (!s_table.eof) {
					char[] l = std.string.stripr(s_table.readLine());
					if (!l.length) continue;
					if (l.length > 11 && l[0..11] == "## POINTER ") {
						pos = toInt(l[11..l.length]);
						while (strings.length <= pos) strings ~= "";
					} else {
						if (strings[pos].length) strings[pos] ~= "\n";
						strings[pos] ~= l;
					}
				}
			}
			//writefln("count: ", strings.length);
		}
		
		read_strings();
		
		//if (s_table) while (!s_table.eof) strings ~= s_table.readLine;
		while (!s_script.eof) {
			char[] line = std.string.strip(s_script.readLine);
			if (!line.length) continue;
			if (line[0] == ':') {
				labels[std.string.strip(line[1..line.length])] = s_out.position;
				continue;
			}
			char[][] tokens = tokenize(line);
			//writefln(tokens);
			OPCODE op = opcodes_name[tokens[0]];
			s_out.write(op.id);
			foreach (k, p; op.params) {
				char[] v = tokens[k + 1];
				switch (p) {
					case 'T': case 'S': case 's':
						if (v.length && v[0] == '$') {
							try {
								v = strings[toInt(v[1..v.length])];
							} catch (Exception e) {
								writefln("Doesn't have ID: %d", toInt(v[1..v.length]));
								v = "";
								//throw(e);
							}
						} else {
							v = v[1..v.length];
						}
						s_out.writeString(std.string.stripr(encodeString(v)) ~ "\0");
					break;
					case '2':
						s_out.write(cast(ushort)toInt(v));
					break;
					case 'P':
						patches ~= PATCH(v, s_out.position);
						s_out.write(cast(uint)0);
					break;
					case 'c':
						s_out.write(cast(ubyte)v[0]);
					break;
					default:
					break;
				}
			}
		}
		
		//foreach (label, v; labels) writefln("LABEL: %s: %08X", label, v);
		
		foreach (patch; patches) {
			//writefln("PATCH: %s: %08X", patch.label, patch.addr);
			s_out.position = patch.addr;
			s_out.write(cast(uint)labels[patch.label]);
		}
	}
	
	static void showAnalytics() {
		writefln("------------------------------------------------");
		writefln(" NAMES                                          ");
		writefln("------------------------------------------------");
		foreach (name; ana_names.keys) {
			writefln("%s", name);
		}

		writefln("------------------------------------------------");
		writefln(" SAVE TITLES                                    ");
		writefln("------------------------------------------------");
		foreach (name; ana_titles) {
			writefln("%s", name);
		}

		writefln("------------------------------------------------");
		writefln(" OPCODES                                        ");
		writefln("------------------------------------------------");
		foreach (op; opcodes) {
			if (!op.count) continue;
			writefln("%02X:%-20s: %d", op.id, op.name, op.count);
		}
		

		writefln("------------------------------------------------");
		writefln(" JUMP_IF OPERATORS                              ");
		writefln("------------------------------------------------");
		foreach (op, count; ana_jump_if_ops) {
			printf("'%c': %d\n", op, count);
		}

		writefln("------------------------------------------------");
		writefln(" SET OPERATORS                                  ");
		writefln("------------------------------------------------");
		foreach (op, count; ana_set_ops) {
			printf("'%c': %d\n", op, count);
		}

		writefln("------------------------------------------------");
		writefln(" USED FLAGS (SET, COMP)                         ");
		writefln("------------------------------------------------");
		bool showPers = false;
		foreach (flag; ana_flags_used.keys.sort) {
			auto count = ana_flags_used[flag];
			if (!showPers && flag > 599) {
				writefln("------------------------------------------------");
				showPers = true;
			}
			printf("$%3d: %3d, %3d", flag, count.set, count.comp);
			if (!count.set ) printf(" -- Not assigned!");
			if (!count.comp) printf(" -- Not compared!");
			printf("\n");
		}

		writefln("------------------------------------------------");
	}
	
	static bool[char[]] ana_names;
	static char[][] ana_titles;
	static int[char] ana_jump_if_ops, ana_set_ops;
	static char[][] ana_links;
	static int ana_end;
	
	struct ana_flags_used_struct {
		int set;
		int comp;
	}
	
	static ana_flags_used_struct[int] ana_flags_used;
	
	static void analyze_string(char[] s) {
		char[][] args = std.string.split(s, "[");
		if (args.length < 2) return;
		args = std.string.split(args[1], "]");
		ana_names[args[0]] = true;
	}

	static void decompile(Stream s, Stream out_s = null, Stream out_t = null, bool acme = false) {
		char[][uint] lines;
		char[][uint] labels;
		char[][] strings;
		
		char[] label(uint addr) {
			if ((addr in labels) is null) labels[addr] = std.string.format("label_%d", labels.length);
			return labels[addr];
		}
		
		ana_links = [];
		ana_end = 0;
	
		while (!s.eof) {
			uint opos = s.position;
			ushort op_id; s.read(op_id);
			if (op_id >= opcodes.length) throw(new Exception(std.string.format("Invalid opcode 0x%02X", op_id)));
			OPCODE op = opcodes[op_id];
			
			// Analyze
			if (!out_s) opcodes[op_id].count++;
			
						
			// END
			if (!out_s && (op.id == 0x19)) {
				ana_end++;
			}
			
			char[] line = op.name;
			
			foreach (k, p; op.params) {
				line ~= (k == 0) ? " " : ", ";
				switch (p) {
					case 'S': case 'T': case 's':
						char[] cs;
						while (true) {
							char v; s.read(v);
							if (v == 0) break;
							cs ~= v;
						}
						
						// Fix BUG in two scripts within opcode 0x47
						if (p == 's') {
							foreach (ck, c; cs) {
								if (c == ',') {
									cs = cs[0..ck + 1];
									s.position = s.position - (cs.length - ck + 1);
									break;
								}
							}
						}
						
						cs = decodeString(cs);
						
						if (!out_s) {
							if (op.id == 0x50) ana_titles ~= cs;
							analyze_string(cs);
						}
						
						if (out_t && p == 'T') {
							line ~= std.string.format("$%d", strings.length);
							strings ~= cs;
						} else {
							line ~= "\"" ~ addslashes(cs) ~ "\"";
						}
						
						// SCRIPT
						if (!out_s && (op.id == 0x18)) ana_links ~= cs;						
					break;
					case 'c': // '=', '+', '-', '{', '}'
						char v; s.read(v);
						
						if (!out_s) {
							if (op.id == 0x04) { // SET
								if ((v in ana_set_ops) is null) ana_set_ops[v] = 0;
								ana_set_ops[v]++;
							} else if (op.id == 0x10) { // JUMP_IF
								if ((v in ana_jump_if_ops) is null) ana_jump_if_ops[v] = 0;
								ana_jump_if_ops[v]++;
							}
						}
						
						line ~= "'" ~ v ~ "'";
					break;
					case '2': 
						ushort v; s.read(v);
						
						if (k == 0) {
							if (op.id == 0x04 || op.id == 0x10) {
								if ((v in ana_flags_used) is null) ana_flags_used[v] = ana_flags_used_struct(0, 0);
								if (op.id == 0x04) {
									ana_flags_used[v].set++;
								} else {
									ana_flags_used[v].comp++;
								}
							}
						}
						
						line ~= std.string.format("%d", v);
					break;
					case 'P':
						uint v; s.read(v);
						line ~= label(v);
					break;
				}
			}
			
			lines[opos] = line;
			//writefln("%s", line);
		}
		
		lines[s.position] = "";
		
		if (out_s) foreach (addr; lines.keys.sort) {
			auto line = lines[addr];
			if (addr in labels) {
				out_s.writefln;
				out_s.writeString(":" ~ labels[addr] ~ "\n");
			}
			out_s.writeString(" " ~ line);
			//out_s.writeString(std.string.format(" # 0x%06X", addr));
			out_s.writefln();
		}
		
		if (out_t) foreach (k, zs; strings) {
			if (acme) out_t.writefln("## POINTER %d", k);
			out_t.writeString(zs ~ "\n");
			if (acme) out_t.writefln;
		}
	}
}
