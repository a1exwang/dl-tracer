import os
import pathlib
import re
import stat
import shutil
import string
import subprocess
import sys
import tempfile
import argparse


def generate_tracer(template, function_name):
    return string.Template(template).substitute(
        function_name=function_name,
        function_end=function_name + "_end",
        function_original=function_name + "_original",
        function_tracer=function_name + "_tracer",
    )


def generate_compile_command(files, output_file):
    includes = ["-I3rdparty/backward-cpp"]
    shared_libs = ["-ldl", "-ldw", "-lbfd", "-lunwind", "-lunwind-x86_64", "-lbacktrace"]
    cmd = ["g++", "-o", output_file, "-fvisibility=hidden", "-shared", "-fPIC", *includes, *files, *shared_libs]
    print(' '.join(cmd))

    return "#!/bin/bash\nset -xue\n%s" % (' '.join(cmd),)


def generate_tracer_library(files, output_file):
    includes = ["-I3rdparty/backward-cpp"]
    shared_libs = ["-ldl", "-ldw", "-lbfd", "-lunwind", "-lunwind-x86_64", "-lbacktrace"]
    cmd = ["g++", "-o", output_file, "-fvisibility=hidden", "-shared", "-fPIC", *includes, *files, *shared_libs]
    print(' '.join(cmd))
    print(subprocess.run(cmd))


def generate_tracer_source(funcs, workdir):
    with open("tracer.S.template", "r") as f:
        asm_template = f.read()
    with open("tracer.cpp.template", "r") as f:
        c_template = f.read()

    files = [os.path.join(os.getcwd(), "stack_dump.cpp"), os.path.join(os.getcwd(), "dl_tracer.cpp")]
    for func in funcs:
        asm_out = os.path.join(workdir, "%s.tracer.S" % func)
        c_out = os.path.join(workdir, "%s.tracer.cpp" % func)
        with open(asm_out, "w") as f:
            f.write(generate_tracer(asm_template, func))
        with open(c_out, "w") as f:
            f.write(generate_tracer(c_template, func))
        files.append(asm_out)
        files.append(c_out)
    print(files)
    return files


if len(sys.argv) <= 2:
    print("Must be at least 1 symbol and 1 output file", file=sys.stderr)
    exit(1)

parser = argparse.ArgumentParser(description='Dynamic Library Tracer Generator')
parser.add_argument('--symbols', type=str, help='Symbol names to trace')
parser.add_argument('--mode', type=str,
                    help='Mode: "so" for generating share library, "src" for generating source code')
parser.add_argument('--output', type=str,
                    help="Output path. A directory for source code generation, a file for so generation")
args = parser.parse_args()

funcs = args.symbols.split(",")
for func in funcs:
    if not re.match(r'^[_A-Za-z][_A-Za-z0-9]*$', func):
        print("Must be valid symbol name, wrong symbol '%s'" % (func,), file=sys.stderr)
        exit(1)

if args.mode == 'so':
    tmp_dir = tempfile.mkdtemp(prefix='generate-tracer-')
    try:
        files = generate_tracer_source(funcs, tmp_dir)
        generate_tracer_library(files, args.output)
    finally:
        shutil.rmtree(tmp_dir)
else:
    output_dir = args.output
    try:
        s = os.stat(output_dir)
        if not stat.S_ISDIR(s.st_mode):
            print("Output file '%s' must be a dir." % output_dir, file=sys.stderr)
            exit(1)
    except FileNotFoundError:
        os.mkdir(output_dir)
        os.chmod(output_dir, 0o755)

    files = generate_tracer_source(funcs, output_dir)
    compile_sh = "compile.sh"
    with open(compile_sh, "w") as f:
        content = generate_compile_command(files, 'libdl_tracer.so')
        f.write(content)
    os.chmod(compile_sh, 0o755)


