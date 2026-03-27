// zig fmt: off

const std = @import("std");
const builtin = @import("builtin");

const isWindows = builtin.os.tag == .windows;

const INSTALLER = if (isWindows) "clisw-installer.exe" else "clisw-installer";
const LAUNCHER = if (isWindows) "clisw-launcher.exe" else "clisw-launcher";
const GAME = if (isWindows) "clisw.exe" else "clisw";

const INSTALLER_DIR = "..";
const DATA_DIR = "data";
const MISC_DIR = "data/misc";
const STORY_DIR = "data/story";
const MAPS_DIR = "data/maps";
const SAVES_DIR = "data/saves";

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
	const url = "https://raw.githubusercontent.com/Mnemos-Parasynthima/TextAdventure-CLISoulWorker/main/version";

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

fn runInstaller(allocator: std.mem.Allocator, opt: Opt) !void {
	// std.debug.print("Changing to installer directory...\n", .{});

	var installerDir = std.fs.cwd().openDir(INSTALLER_DIR, .{}) catch |err| {
		// std.debug.print("Could not open installer directory: {}\n", .{err});
		try stderr.print("Could not open installer directory: {}\n", .{err});
		std.process.exit(BAD_EXIT);
	};
	defer installerDir.close();
	installerDir.setAsCwd() catch |err| {
		// std.debug.print("Could not change to installer directory: {}\n", .{err});
		try stderr.print("Could not change to installer directory: {}\n", .{err});
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

pub fn main() !void {
	var gpa = std.heap.GeneralPurposeAllocator(.{}).init;
	defer _ = gpa.deinit();
	const allocator = gpa.allocator();

	// check existence of version file
	const fs = std.fs.cwd();
	const vf = fs.openFile("version", .{ .mode = .read_only }) catch {
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

	const gameFile = fs.openFile(GAME, .{ .mode = .read_only }) catch {
		try stdout.print("Could not find game! Fixing files...\n", .{});
		try stdout.flush();
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	};
	gameFile.close();

	if (fs.statFile(DATA_DIR)) |s| {
		if (s.kind != .directory) {
			try stdout.print("Data path exists but is not a directory! Fixing files...\n", .{});
			try stdout.flush();
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find data directory! Fixing files...\n", .{});
		try stdout.flush();
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}


	if (fs.statFile(MAPS_DIR)) |s| {
		if (s.kind != .directory) {
			try stdout.print("Maps path exists but is not a directory! Fixing files...\n", .{});
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find maps directory! Fixing files...\n", .{});
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}

	if (fs.statFile(MISC_DIR)) |s| {
		if (s.kind != .directory) {
			try stdout.print("Misc path exists but is not a directory! Fixing files...\n", .{});
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find misc directory! Fixing files...\n", .{});
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}

	if (fs.statFile(STORY_DIR)) |s| {
		if (s.kind != .directory) {
			try stdout.print("Story path exists but is not a directory! Fixing files...\n", .{});
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find story directory! Fixing files...\n", .{});
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}

	if (fs.statFile("data/story/control_zone")) |s| {
		if (s.kind != .directory) {
			try stdout.print("Control zone path exists but is not a directory! Fixing files...\n", .{});
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find control zone directory! Fixing files...\n", .{});
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}

	if (fs.statFile("data/story/r_square")) |s| {
		if (s.kind != .directory) {
			try stdout.print("R Square path exists but is not a directory! Fixing files...\n", .{});
			runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
			return;
		}
	} else |_| {
		try stdout.print("Could not find R Square directory! Fixing files...\n", .{});
		runInstaller(allocator, Opt.FIX) catch { std.process.exit(BAD_EXIT); };
		return;
	}

	if (fs.statFile(SAVES_DIR)) |s| {
		if (s.kind != .directory) {
			try stdout.print("Saves path exists but is not a directory! Creating saves directory...\n", .{});
			// Remove the file and create the directory
			fs.deleteFile(SAVES_DIR) catch |err| {
				try stdout.print("Could not delete invalid saves file: {}\n", .{err});
				std.process.exit(BAD_EXIT);
			};
			fs.makeDir(SAVES_DIR) catch |err| {
				try stdout.print("Could not create saves directory: {}\n", .{err});
				std.process.exit(BAD_EXIT);
			};
		}
	} else |_| {
		try stdout.print("Could not find saves directory! Creating saves directory...\n", .{});
		fs.makeDir(SAVES_DIR) catch |err| {
			try stdout.print("Could not create saves directory: {}\n", .{err});
			std.process.exit(BAD_EXIT);
		};
	}

	try stdout.print("Verification done. Game starting...\n", .{});
	try stdout.flush();
	ssleep(1000);

	const launcherFlag = "-l";

	const cwd = try std.fs.selfExeDirPathAlloc(allocator);
	defer allocator.free(cwd);

	const gameExe = try std.fs.path.join(allocator, &[_][]const u8{cwd, GAME});
	defer allocator.free(gameExe);
	const gameArgs = &[_][]const u8{gameExe, launcherFlag};

	var proc = std.process.Child.init(gameArgs, allocator);
	proc.cwd = cwd;
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