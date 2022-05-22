from collections import Counter

def main(filepath):
    counter = Counter()
    with open(filepath, 'r', encoding='iso-8859-15') as file:
        for line in file:
            counter.update(line.split(' '))
    return counter            

if __name__ == '__main__':
    main('./gutenberg-500M.txt')