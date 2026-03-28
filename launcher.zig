// zig fmt: off

const std = @import("std");
const builtin = @import("builtin");

const isWindows = builtin.os.tag == .windows;

const SEP: []const u8 = if (isWindows) "\\" else "/";

const INSTALLER = if (isWindows) "clisw-installer.exe" else "clisw-installer";
const LAUNCHER = if (isWindows) "clisw-launcher.exe" else "clisw-launcher";
const GAME = if (isWindows) "clisw.exe" else "clisw";

const INSTALLER_DIR = "..";
const DATA_DIR = "data";

// Array of data directories
const DATA_DIRS = &[_][]const u8{
	"misc",
	"maps",
	"saves",
	"story",
	"story" ++ SEP ++ "best_showtime",
	"story" ++ SEP ++ "control_zone",
	"story" ++ SEP ++ "r_square",
	"story" ++ SEP ++ "tower_of_greed"
};

const BAD_EXIT: i32 = 0xff;

var outBuffer: [1024]u8 = undefined;
var errBuffer: [1024]u8 = undefined;
var w: std.fs.File.Writer = switch (builtin.os.tag) {
	.windows => undefined,
	else => std.fs.File.stdout().writer(&outBuffer),
};
var e: std.fs.File.Writer = switch (builtin.os.tag) {
	.windows => undefined,
	else => std.fs.File.stderr().writer(&errBuffer),
};
const stdout = &w.interface;
const stderr = &e.interface;


fn ssleep(ms: u64) void {
	std.Thread.sleep(ms * std.time.ns_per_ms);
}

fn trimNewline(s: []u8) []u8 {
	var end = s.len;
	while (end > 0) : (end -= 1) {
		const c = s[end - 1];
		if (c == '\n' or c == '\r') continue;
		break;
	}
	return s[0..end];
}

fn checkLatest(allocator: std.mem.Allocator) !bool {
	const url = "https://raw.githubusercontent.com/jgarcdev/TextAdventure-CLISoulWorker/main/version";

	var client = std.http.Client{ .allocator = allocator };
	defer client.deinit();

	const uri = try std.Uri.parse(url);

	var resBuffer: [64]u8 = undefined;
	var responseWriter = std.io.Writer.fixed(&resBuffer);


	const res = client.fetch(.{
		.location = .{ .uri = uri },
		.redirect_behavior = .init(1),
		.response_writer = &responseWriter,
		.method = .GET,
		.headers = .{
			.user_agent = .{ .override = "VersionChecker/1.0" },
		}
	}) catch |err| {
		// std.debug.print("HTTP request failed: {}\n", .{err});
		try stderr.print("HTTP request failed: {}\n", .{err});
		try stderr.flush();
		return true;
	};

	if (res.status != .ok) {
		// std.debug.print("HTTP request returned non-OK status: {}\n", .{res.status});
		try stderr.print("HTTP request returned non-OK status: {}\n", .{res.status});
		try stderr.flush();
		return true;
	}
	const body = trimNewline(resBuffer[0..responseWriter.end]);

	// std.debug.print("Response: {s}\n", .{body});


	const localFile = std.fs.cwd().openFile("version", .{ .mode = .read_only }) catch |err| {
		// std.debug.print("Could not open local version file: {}\n", .{err});
		try stderr.print("Could not open local version file: {}\n", .{err});
		try stderr.flush();
		return true;
	};
	defer localFile.close();


	var localBuf: [32]u8 = undefined;
	const readBytes = localFile.read(&localBuf) catch |err| {
		std.debug.print("Could not read local version file: {}\n", .{err});
		try stderr.print("Could not read local version file: {}\n", .{err});
		try stderr.flush();
		return true;
	};

	const local = trimNewline(localBuf[0..readBytes]);

	try stdout.print("Checking version...\nCurrent version is {s}\nLatest version is {s}\n", .{local, body});
	try stdout.flush();

	return std.mem.eql(u8, body, local);
}

const Opt = enum { FIX, UPDATE };

fn runInstaller(allocator: std.mem.Allocator, opt: Opt) !noreturn {
	// std.debug.print("Changing to installer directory...\n", .{});

	var installerDir = std.fs.cwd().openDir(INSTALLER_DIR, .{}) catch |err| {
		// std.debug.print("Could not open installer directory: {}\n", .{err});
		try stderr.print("Could not open installer directory: {}\n", .{err});
		try stderr.flush();
		std.process.exit(BAD_EXIT);
	};
	defer installerDir.close();
	installerDir.setAsCwd() catch |err| {
		// std.debug.print("Could not change to installer directory: {}\n", .{err});
		try stderr.print("Could not change to installer directory: {}\n", .{err});
		try stderr.flush();
		std.process.exit(BAD_EXIT);
	};

	ssleep(1000);

	const args = switch (opt) {
		Opt.FIX => &[_][]const u8{INSTALLER, "-f"},
		Opt.UPDATE => &[_][]const u8{INSTALLER, "-u"},
	};

	var proc = std.process.Child.init(args, allocator);
	try proc.spawn();

	const exitCode = proc.wait() catch |err| {
		try stdout.print("Could not execute installer: {}\n", .{err});
		try stdout.flush();
		std.process.exit(BAD_EXIT);
	};
	if (exitCode.Exited != 0) {
		try stderr.print("Installer exited with code {}\n", .{exitCode});
		try stderr.flush();
		std.process.exit(BAD_EXIT);
	}
	std.process.exit(0);
}

fn verifyDataDir(allocator: std.mem.Allocator, cwd: std.fs.Dir) !void {
	var dataDir = cwd.openDir(DATA_DIR, .{.iterate = true}) catch |err| switch (err) {
		error.FileNotFound => {
			try stdout.print("Could not find data! Reinstalling...\n", .{});
			try stdout.flush();
			try runInstaller(allocator, .FIX);
			unreachable;
		},
		error.NotDir => {
			try stderr.print("Data is not a directory! Reinstalling...\n", .{});
			try runInstaller(allocator, .FIX);
			unreachable;
		},
		else => {
			try stderr.print("Could not open data directory: {}\n", .{err});
			std.process.exit(BAD_EXIT);
		}
	};
	defer dataDir.close();

	var dataDirWalker = try dataDir.walk(allocator);
	defer dataDirWalker.deinit();

	const numDirs = DATA_DIRS.len;
	var found: [numDirs]bool = undefined;
	for (0..numDirs) |i| {
		found[i] = false;
	}

	while (try dataDirWalker.next()) |entry| {
		if (entry.kind != .directory) continue;
		for (0..numDirs) |i| {
			const dir = DATA_DIRS[i];
			if (std.mem.eql(u8, entry.path, dir)) {
				std.debug.print("Found data directory: {s} from {s}\n", .{dir, entry.path});
				found[i] = true;
				break;
			}
		}
	}

	if (!found[2]) {
		// Not finding saves/ is not an error, just make it
		const savesFullPath = try std.fs.path.join(allocator, &[_][]const u8{DATA_DIR, DATA_DIRS[2]});
		cwd.makeDir(savesFullPath) catch |err| {
			try stderr.print("Could not create saves directory: {}\n", .{err});
			std.process.exit(BAD_EXIT);
		};
		found[2] = true;
	}

	var missing = false;
	for (0..numDirs) |i| {
		const dir = DATA_DIRS[i];
		if (!found[i]) {
			if (!missing) try stderr.print("Missing required data directories:\n", .{});
			missing = true;
			try stderr.print(" - {s}\n", .{dir});
		}
	}
	if (missing) {
		try stderr.flush();
		runInstaller(allocator, .FIX) catch { std.process.exit(BAD_EXIT); };
		unreachable;
	}
}

pub fn main() !void {
	var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
	defer _ = gpa.deinit();
	const allocator = gpa.allocator();

	switch (builtin.os.tag) {
		.windows => {
			w = std.fs.File.stdout().writer(&outBuffer);
			e = std.fs.File.stderr().writer(&errBuffer);
		},
		else => {}
	}

	// check existence of version file
	const cwd = std.fs.cwd();
	const vf = cwd.openFile("version", .{ .mode = .read_only }) catch {
		// std.debug.print("Could not find version file! Fixing files...\n", .{});
		try stderr.print("Could not find version file! Fixing files...\n", .{});
		try stderr.flush();
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	};
	vf.close();

	ssleep(1000);

	const latest = try checkLatest(allocator);
	if (!latest) {
		var rBuffer: [1024]u8 = undefined;
		var r: std.fs.File.Reader = std.fs.File.stdin().reader(&rBuffer);
		const stdin = &r.interface;

		try stdout.print("New version found! Do you want to update? [y|n] \n", .{});
		try stdout.flush();
		var res = stdin.takeByte() catch |err| {
			try stderr.print("Failed reading response: {}\n", .{err});
			return;
		};

		if (res >= 0x41 and res <= 0x5A) res += 32;
		if (res == 'y') {
			runInstaller(allocator, Opt.UPDATE) catch { std.process.exit(BAD_EXIT); };
			return;
		} else {
			try stdout.print("Will not update. It is highly recommended to update for latest features and fixes!\n", .{});
		}
	}

	try stdout.print("Verifying files and data...\n", .{});
	ssleep(1000);

	const gameFile = cwd.openFile(GAME, .{ .mode = .read_only }) catch {
		try stdout.print("Could not find game! Fixing files...\n", .{});
		try stdout.flush();
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	};
	gameFile.close();

	try verifyDataDir(allocator, cwd);

	try stdout.print("Verification done. Game starting...\n", .{});
	try stdout.flush();
	ssleep(1000);

	const launcherFlag = "-l";

	const cwdStr = try std.fs.selfExeDirPathAlloc(allocator);
	defer allocator.free(cwdStr);

	const gameExe = try std.fs.path.join(allocator, &[_][]const u8{cwdStr, GAME});
	defer allocator.free(gameExe);
	const gameArgs = &[_][]const u8{gameExe, launcherFlag};

	var proc = std.process.Child.init(gameArgs, allocator);
	proc.cwd = cwdStr;
	// if (proc.cwd) |_cwd| {
	// std.debug.print("Setting working directory to {s}\n", .{_cwd});
	// } else {
	// std.debug.print("No working directory set for child process\n", .{});
	// }
	const exitCode = proc.spawnAndWait() catch |err| {
		try stdout.print("Could not start game: {}\n", .{err});
		try stdout.flush();
		std.process.exit(BAD_EXIT);
	};

	if (exitCode.Exited != 0) {
		try stdout.print("Game exited with code {}\n", .{exitCode});
		try stdout.flush();
		std.process.exit(BAD_EXIT);
	}
}