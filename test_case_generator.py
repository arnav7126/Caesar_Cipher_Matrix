import random
import string

def decode(encoded_string, key):
    key = key % 26
    decoded = ""
    for c in encoded_string:
        num = ord(c) + key
        if num > 122:
            num -= 26
        decoded += chr(num)
    return decoded

def encode(decoded_string, key):
    key = key % 26
    key = (26 - key % 26) % 26
    return decode(decoded_string, key)

def get_standard_permutation(word):
    diff = ord(word[0]) - ord("a")
    return encode(word, diff)

# Generate 10 test cases with increasing difficulty
for testcase_number in range(1, 21):

    # Increase difficulty by increasing size, word count, and word length
    n = 5 + (testcase_number*5)  # Increase the matrix size each test case
    min_word_length = 5 + (testcase_number*5)  # Longer words
    max_word_length = 20 + (testcase_number*5)  # Even longer words
    word_count = 500 + (testcase_number * 200)  # More words

    matrix_set = set()
    words = []
    standard_permutation_set = set()

    while len(matrix_set) < n * n:
        length = random.randint(min_word_length, max_word_length)
        word = "".join(random.choices(string.ascii_lowercase, k=length))

        standard_permutation = get_standard_permutation(word)
        if standard_permutation in standard_permutation_set:
            continue

        matrix_set.add(word)
        standard_permutation_set.add(standard_permutation)

    max_occurrences = word_count // (n * n)
    matrix_list = list(matrix_set)
    matrix_answers = {}

    for word in matrix_list:
        occurrences = random.randint(0, max_occurrences)
        matrix_answers[word] = occurrences
        words.extend([word] * occurrences)

    while len(words) < word_count:
        if random.random() > 0.9:
            length = random.randint(min_word_length, max_word_length)
            word_is_viable = False
            while not word_is_viable:
                word = "".join(random.choices(string.ascii_lowercase, k=length))
                if word not in matrix_set:
                    word_is_viable = True
        else:
            word = decode(random.choice(matrix_list), random.randint(1, 25))

        words.append(word)

    random.shuffle(words)

    with open(f"words{testcase_number}.txt", "w") as output_file:
        output_file.write(" ".join(word for word in words))

    matrix = []
    for i in range(n):
        row = []
        for j in range(n):
            row.append(matrix_list[j + i * n])
        matrix.append(row)

    diagonal_answers = []
    for i in range(2 * n - 1):
        col = min(i, n - 1)
        row = max(0, i - n + 1)

        diagonal_sum = 0
        while col >= 0 and row < n:
            diagonal_sum += matrix_answers[matrix[row][col]]
            col -= 1
            row += 1

        diagonal_answers.append(diagonal_sum)

    encoded_matrix = [[None for _ in range(n)] for _ in range(n)]
    encoded_matrix[0][0] = matrix[0][0]
    keys = []

    for i in range(1, 2 * n - 1):
        key = random.randint(1000 * testcase_number, 100000 * testcase_number)  # Harder keys
        keys.append(key)

        col = min(i, n - 1)
        row = max(0, i - n + 1)

        while col >= 0 and row < n:
            encoded_matrix[row][col] = encode(matrix[row][col], key)
            col -= 1
            row += 1

    with open(f"testcase{testcase_number}.bin", "w") as testcase_file:
        testcase_file.write(f"{n} {max_word_length + 1}\n")  
        for i in range(n):
            testcase_file.write(" ".join(word for word in encoded_matrix[i]) + "\n")

    with open(f"answer{testcase_number}.bin", "w") as answer_file:
        answer_file.write(f"{len(diagonal_answers)}\n")
        answer_file.write(" ".join(str(answer) for answer in diagonal_answers) + "\n")
        answer_file.write(" ".join(str(key) for key in keys) + " 0")

    print(f"✅ Test Case {testcase_number} generated. (n={n}, words={word_count}, word_len={min_word_length}-{max_word_length})")
