
import os

def elements_in_file(file_path):
    try:
        with open(file_path, 'r') as file:
            for line in file:
                return list(map(int, line.strip().split(' ')))
    except FileNotFoundError:
        print(f"File {file_path} not found.")
        return 0


def main():
    base_path = '../data/local'
    a = []
    for i in range(1, 9):
        file_path = f"{base_path}{i}/file{i}"
        elements = elements_in_file(file_path)
        for x in elements:
            a.append(x)

    b = sorted(a)
    for i in range(len(a)):
        if a[i] != b[i]:
            print("Test is not passed")
            break
    print("Test passed")


if __name__ == "__main__":
    main()
