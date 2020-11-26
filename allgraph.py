# pylint: disable-all
import os
from pathlib import Path
from typing import Tuple, List, Iterator
 
from graf2020.dimacs2 import loadDirectedWeightedGraph
 
 
def read_graph(path: Path) -> Tuple[int, List[Tuple[int, int, int]]]:
    V, E = loadDirectedWeightedGraph(path)
    E2 = []
    for u, v, c in E:
        E2.append((u - 1, v - 1, c))
    return V, E2
 
 
def read_all_graphs_with_solutions_gen(base_dir: Path) -> Iterator[Tuple[str, int, Tuple[int, List[Tuple[int, int, int]]]]]:
    (_, _, filenames) = next(os.walk(str(base_dir)))
    for filename in filenames:
        filepath = base_dir / filename
        G = read_graph(path=filepath)
        expected_solution = int(filepath.read_text().splitlines()[0].split()[-1].strip())
        yield filename, expected_solution, G