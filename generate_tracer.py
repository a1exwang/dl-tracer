import os
import re
import shutil
import string
import subprocess
import sys
import tempfile


def generate_tracer(template, function_name):
    return string.Template(template).substitute(
        function_name=function_name,
        function_end=function_name + "_end",
        function_original=function_name + "_original",
    )


def generate_tracer_library(funcs, output_file):
    with open("tracer.S.template", "r") as f:
        asm_template = f.read()
    with open("tracer.cpp.template", "r") as f:
        c_template = f.read()

    tmp_dir = tempfile.mkdtemp(prefix='generate-tracer-')
    try:
        files = ["stack_dump.cpp", "dl_tracer.cpp"]
        includes = ["-I3rdparty/backward-cpp"]
        shared_libs = ["-ldl", "-ldw", "-lbfd", "-lunwind", "-lunwind-x86_64", "-lbacktrace"]
        for func in funcs:
            asm_out = os.path.join(tmp_dir, "%s.tracer.S" % func)
            c_out = os.path.join(tmp_dir, "%s.tracer.cpp" % func)
            with open(asm_out, "w") as f:
                f.write(generate_tracer(asm_template, func))
            with open(c_out, "w") as f:
                f.write(generate_tracer(c_template, func))

            files.append(asm_out)
            files.append(c_out)

        print(files)
        cmd = ["g++", "-o", output_file, "-fvisibility=hidden", "-shared", "-fPIC", *includes, *files, *shared_libs]
        print(' '.join(cmd))
        print(subprocess.run(cmd))
    finally:
        shutil.rmtree(tmp_dir)


if len(sys.argv) <= 2:
    print("Must be at least 1 symbol and 1 output file", file=sys.stderr)
    exit(1)

output_file = sys.argv[1]
funcs = sys.argv[2:]
for func in funcs:
    if not re.match(r'^[_A-Za-z][_A-Za-z0-9]*$', func):
        print("Must be valid symbol name, wrong symbol '%s'" % (func,), file=sys.stderr)
        exit(1)
generate_tracer_library(funcs, output_file)
