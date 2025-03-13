import math

with open("results/key_ranks.txt", "r") as f:
    guessing_entropy_list = f.readlines()

guessing_entropy = sum([math.log2(float(x)) for x in guessing_entropy_list])
print(f"Guessing Entropy: {guessing_entropy}")
