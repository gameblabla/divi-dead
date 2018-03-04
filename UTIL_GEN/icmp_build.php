<?php
	$f = fopen("../ICMP.DAT", "wb");
	$count = 48;
	$cur = 4 + $count * 8;
	fwrite($f, pack('V', $count));
	
	function putData($data) {
		global $f, $cur;
		$size = strlen($data);
		$rcur = ftell($f);
		fseek($f, $cur);
		fwrite($f, $data);
		fseek($f, $rcur);
		fwrite($f, pack('V', $size ? $cur : 0));
		fwrite($f, pack('V', $size));
		$cur += $size;
	}
	
	for ($n = 1; $n <= 13; $n++) {
		putData($d = @file_get_contents(sprintf('B%02d_C.png', $n)));
		putData($d = @file_get_contents(sprintf('B%02d_M.png', $n)));
		putData($d = @file_get_contents(sprintf('B%02dA_C.png', $n)));
		putData($d = @file_get_contents(sprintf('B%02dA_M.png', $n)));
	}
?>