from ortools.graph import pywrapgraph
import os, sys

def ceildiv(a, b):
    return -(-a // b)

def n_choose_two(n):
    return n * (n - 1) // 2

def solve(test):
    n = test[0]
    budget = test[1]
    matches = test[2]
    won_by_king = sum([1 for match in matches if match[0] == 0])

    can_win = False
    for desired_score in range(max(ceildiv(n, 2), won_by_king), n):
        solver = pywrapgraph.SimpleMinCostFlow()

        matches_from = n
        matches_to = matches_from + n_choose_two(n)

        s = matches_to
        t = s + 1
        bottleneck = t + 1

        for i in range(matches_from, matches_to):
            solver.AddArcWithCapacityAndUnitCost(s, i, 1, 0)

        for i in range(matches_from, matches_to):
            match = matches[i - matches_from]
            solver.AddArcWithCapacityAndUnitCost(i, match[0], 1, 0)
            solver.AddArcWithCapacityAndUnitCost(i, match[1], 1, match[2])

        solver.AddArcWithCapacityAndUnitCost(0, t, desired_score, 0)

        for i in range(1, n):
            solver.AddArcWithCapacityAndUnitCost(i, bottleneck, desired_score, 0)

        solver.AddArcWithCapacityAndUnitCost(bottleneck, t, n_choose_two(n) - desired_score, 0)

        solver.SetNodeSupply(s, n_choose_two(n))
        solver.SetNodeSupply(t, -1 * n_choose_two(n))

        status = solver.Solve()

        if (solver.OptimalCost() <= budget):
            can_win = True

        print(f"n: {n}, b: {budget}, x: {desired_score}, st: {status}, cost: {solver.OptimalCost()}")

    return can_win

def extract_numbers(line):
    return [int(x) for x in line.split()]

def main():
    input_data = [extract_numbers(x) for x in open("basictest.txt", "r").readlines()]

    tests = []
    line_no = 1

    while line_no < len(input_data):
        budget = input_data[line_no][0]
        line_no += 1
        n_players = input_data[line_no][0]
        line_no += 1

        matches = []
        for _ in range(n_choose_two(n_players)):
            line = input_data[line_no]
            p1 = line[0]
            p2 = line[1]
            winner = line[2]
            bribe_cost = line[3]

            if winner == p2:
                p1, p2 = p2, p1

            matches.append((p1, p2, bribe_cost))

            line_no += 1

        tests.append((n_players, budget, matches))

    print([solve(test) for test in tests])

if __name__ == "__main__":
    os.chdir(sys.path[0])
    main()