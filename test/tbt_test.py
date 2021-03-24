import subprocess
import os


def rm_file(file):
    if os.path.exists(file):
        print("[+] rm " + file)
        os.remove(file)


def test(command, check_returncode=0, check_stderr=False, file_exist=None):
    proc = subprocess.Popen(
        command.split(' '), stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    proc.wait()

    check_test = True

    print("[+] " + command)

    if file_exist != None:
        print("File " + file_exist + " created :", end='')
        if os.path.exists(file_exist):
            print(" yes (passed)")
        else:
            print(" no (failed)")
            check_test = False

    print("Return Code : " + str(proc.returncode), end='')
    if check_returncode == 0 and proc.returncode == 0:
        print(" (passed)")
    elif check_returncode != 0 and proc.returncode != 0:
        print(" (passed)")
    else:
        print(" (failed)")
        check_test = False

    if check_stderr:
        print("Stderr written: ", end='')
        if len(proc.stderr.read().strip()) != 0:
            print("yes (passed)")
        else:
            print("no (failed)")
            check_test = False
    print()

    return check_test


def main():
    check = True

    print("\n-----( Test )-----")
    check &= test("../tbt ./ -o all_test", file_exist="all_test")
    check &= test("../tbt . -a CTPH -o ctph_H_test", file_exist="ctph_H_test")
    check &= test("../tbt . -a SIMHASH -o simhash_H_test",
                  file_exist="simhash_H_test")
    check &= test("../tbt -c ctph_H_test -a SIMHASH",
                  check_returncode=-1, check_stderr=True)
    check &= test("../tbt -c simhash_H_test -a CTPH",
                  check_returncode=-1, check_stderr=True)
    check &= test("../tbt -c all_test -o only_CTPh -a CTPH",
                  file_exist="only_CTPh")
    check &= test("../tbt -c all_test -o only_SIMh -a SIMHASH",
                  file_exist="only_SIMh")
    check &= test("../tbt -c all_test -o a_test", file_exist="a_test")
    check &= test("../tbt -c ctph_H_test -o c_test", file_exist="c_test")
    check &= test("../tbt -c simhash_H_test -o s_test", file_exist="s_test")

    if check:
        print("[!] All tests passed")
    else:
        print("[!] Some tests failed")

    # Cleaning created files
    print("\n-----( Cleaning )-----")
    rm_file('all_test')
    rm_file('ctph_H_test')
    rm_file('simhash_H_test')
    rm_file('only_CTPh')
    rm_file('only_SIMh')
    rm_file('a_test')
    rm_file('c_test')
    rm_file('s_test')


if __name__ == "__main__":
    main()
