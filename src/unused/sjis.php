<?php
	printf("typedef struct { unsigned short sjis, unicode; } sjis_transform;\n\n");
	printf("sjis_transform sjis_transform_table[] = {\n");
	$a = array();
	foreach (file('sjis.txt') as $l) { $l = trim($l);
		if (!strlen($l)) continue;
		$p = explode("\t", $l, 3);
		if (!isset($p[1])) continue;
		if (substr($p[0], 0, 1) == '#') continue;
		if (substr($p[1], 0, 1) == '#') continue;
		if (!strlen($p[1])) continue;
		if (substr($p[1], 0, 2) != 'U+') {
			print_r($p);
			throw(new Exception("Error!"));
		}
		
		$v1 = $p[0];
		$v1 = substr($v1, 4, 2) . substr($v1, 2, 2);
		
		$v1 = hexdec($v1);
		$v2 = hexdec(substr($p[1], 2));
		$a[] = array($v1, $v2);
		//printf("\t{0x%04X, 0x%04X},\n", $v1, $v2);
		//echo "$v1, $v2\n";
	}
	
	function cmp($l, $r) {
		$l = $l[0]; $r = $r[0];
		if ($l == $r) return 0;
		return ($l < $r) ? -1 : 1;
	}
	
	usort($a, 'cmp');
	
	foreach ($a as $v) {
		printf("\t{0x%04X, 0x%04X},\n", $v[0], $v[1]);
	}
	printf("};");
?>