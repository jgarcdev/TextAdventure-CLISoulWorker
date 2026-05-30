// zig fmt: off

const std = @import("std");

pub fn build(b: *std.Build) void {
	const target = b.standardTargetOptions(.{});
	const optimize = b.standardOptimizeOption(.{});

	const dvuiMod = b.dependency("dvui", .{
		.target = target,
		.optimize = optimize,
		.backend = .sdl3
	});


	const exe = b.addExecutable(.{
		.name = "create",
		.root_module = b.addModule("create", .{
			.target = target,
			.root_source_file = b.path("create.zig"),
			.optimize = optimize,
		}),
		// .use_llvm = true
	});

	exe.root_module.addImport("dvui", dvuiMod.module("dvui_sdl3"));

	const installExe = b.addInstallArtifact(exe, .{
		.dest_dir = .{ .override = .{ .custom = "../out/" } }
	});

	b.step("create", "Build the create executable").dependOn(&installExe.step);
}