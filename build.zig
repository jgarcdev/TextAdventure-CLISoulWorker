// zig fmt: off

const std = @import("std");

pub fn build(b: *std.Build) void {
  const target = b.standardTargetOptions(.{});
  const optimize = b.standardOptimizeOption(.{});


  const exe = b.addExecutable(.{
    .name = "clisw",
    .root_module = b.addModule("clisw", .{
      .target = target,
      .optimize = optimize,
      // .sanitize_c = .full
    })
  });

  exe.addCSourceFiles(.{
    .files = &[_][]const u8{
      "external/cJSON.c",
      "external/getline.c",
      "main.c",
      "structures/RoomTable.c",
      "structures/SoulWorker.c",
      "structures/Maze.c",
      "structures/DArray.c",
      "helpers/Error.c",
      "helpers/Misc.c",
      "control/Setup.c",
      "control/Keyboard.c",
      "control/SaveLoad.c",
      "control/Battle.c",
    },
    .flags = &[_][]const u8{
      "-std=c11",
      "-D_CRT_SECURE_NO_WARNINGS",
    },
  });

  exe.addIncludePath(b.path("headers"));

  exe.addWin32ResourceFile(.{
    .file = b.path("resources/clisw.rc"),
  });

  exe.linkLibC();

  const installExe = b.addInstallArtifact(exe, .{
    .dest_dir = .{
      .override = .{.custom = "../build"}
    }
  });
  b.getInstallStep().dependOn(&installExe.step);
}