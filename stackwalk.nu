#!/usr/bin/env nu

def main [] {
	print "Use a subcomand"
}

def "main android64" [] {
	run "./build-android64" "./kevadroz.practicecheckpointpermanence.android64.so"
}

def run [
	build_dir: string,
	binary: string,
] {
	let dump: string = try { zenity --file-selection --title "Select dump" } catch {
		print "Cancelled"
		return
	}

	try { cd $build_dir } catch {
		print "Couldn't open build directory"
		return
	}

	print "Extracting symbols..."
	dump_syms $binary | save "symbols" -f

	print "Walking..."
	minidump-stackwalk --symbols-url https://symbols.xyze.dev/ --symbols-path ./symbols $dump | save "dump.txt" -f

	start "dump.txt"
}
