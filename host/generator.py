
import os
import random

BASE_DIRECTORY = "../data"
NUM_FILES = 8
NUMBERS_PER_FILE = 100000
RANDOM_LOWER_BOUND = -2**31
RANDOM_UPPER_BOUND = 2**31 - 1

def generate_random_numbers(n, lower_bound, upper_bound):
    return [random.randint(lower_bound, upper_bound) for _ in range(n)]

def main():
    # Ensure the base directory exists
    os.makedirs(BASE_DIRECTORY, exist_ok=True)

    for i in range(1, NUM_FILES + 1):
        directory = f"{BASE_DIRECTORY}/local{i}"
        os.makedirs(directory, exist_ok=True)  # Create directory if it doesn't exist
        file_path = f"{directory}/file{i}"

        # Generate random numbers
        numbers = generate_random_numbers(NUMBERS_PER_FILE, RANDOM_LOWER_BOUND, RANDOM_UPPER_BOUND)

        # Write numbers to file
        with open(file_path, 'w') as file:
            for number in numbers:
                file.write(f"{number}\n")

        print(f"Created {file_path} with {NUMBERS_PER_FILE} random numbers.")

if __name__ == "__main__":
    main()
