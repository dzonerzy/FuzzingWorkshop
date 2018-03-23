import lief, sys

if len(sys.argv) < 3:
    print("[-] invalid syntax")
    exit(1)

infile = sys.argv[1]
elf = lief.parse(infile)

for arg in sys.argv[2:]:
    addr, name = arg.split(":", 1)
    addr = int(addr, 16)
    print("[+] exporting '%s' to %#x" % (name, addr,))
    elf.add_exported_function(addr, name)

outfile = "%s.so" % infile
print("[+] writing shared object as '%s'" % (outfile,))
elf.write(outfile)
print("[+] done")
