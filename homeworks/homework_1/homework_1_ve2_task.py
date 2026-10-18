import sys
from enum import IntEnum

sys.setrecursionlimit(1_000_000)


class StateNodeType(IntEnum):
    START = 0
    END = 1


class Direction(IntEnum):
    RIGHT = 0
    LEFT = 1

    def opposite(self) -> "Direction":
        return Direction.LEFT if self == Direction.RIGHT else Direction.RIGHT


class StateNode:
    def __init__(self, dataState: list = None, separatorIndex: int = 0,
                 type: StateNodeType = StateNodeType.START):
        self.separatorIndex = separatorIndex
        self.dataState = dataState if dataState is not None else []
        self.type = type

    @classmethod
    def create(cls, dataState: list, separatorIndex: int, type: StateNodeType) -> "StateNode":
        return cls(dataState, separatorIndex, type)

    def modify(self, leftIndex: int, rightIndex: int) -> None:
        if leftIndex > rightIndex:
            leftIndex, rightIndex = rightIndex, leftIndex
        if rightIndex >= len(self.dataState):
            raise IndexError("Invalid position.")
        if self.separatorIndex == leftIndex:
            self.separatorIndex = rightIndex
        elif self.separatorIndex == rightIndex:
            self.separatorIndex = leftIndex
        self.dataState[leftIndex], self.dataState[rightIndex] = self.dataState[rightIndex], self.dataState[leftIndex]

    def __str__(self) -> str:
        return "".join(self.dataState)


class CONSTANTS:
    LEFT_ARROW = '<'
    RIGHT_ARROW = '>'
    SEPARATOR = '_'


class BUILD:
    @staticmethod
    def buildNode(N: int, type: StateNodeType) -> StateNode:
        resultNodeState = ""
        if type == StateNodeType.START:
            for _ in range(N):
                resultNodeState += CONSTANTS.RIGHT_ARROW
            resultNodeState += CONSTANTS.SEPARATOR
            for _ in range(N):
                resultNodeState += CONSTANTS.LEFT_ARROW
        elif type == StateNodeType.END:
            for _ in range(N):
                resultNodeState += CONSTANTS.LEFT_ARROW
            resultNodeState += CONSTANTS.SEPARATOR
            for _ in range(N):
                resultNodeState += CONSTANTS.RIGHT_ARROW
        else:
            raise RuntimeError("Invalid input data")

        return StateNode.create(list(resultNodeState), N, type)


class RuleBasedSolver:
    def __init__(self, stateNode: StateNode, N: int):
        self.stateNode = stateNode
        self.N = N

    def generateMoveGroups(self) -> list:
        N = self.N
        counts = list(range(1, N + 1)) + [N, N] + list(range(N - 1, 0, -1))
        direction = Direction.RIGHT
        result = []
        for count in counts:
            result.append((direction, count))
            direction = direction.opposite()
        return result

    def applyNextMove(self, direction: Direction) -> None:
        node = self.stateNode
        blank = node.separatorIndex
        size = len(node.dataState)

        if direction == Direction.RIGHT:
            # Jump: right-facing frog leaps two positions over the opposite-facing frog in between
            if (blank >= 2 and
                    node.dataState[blank - 2] == CONSTANTS.RIGHT_ARROW and
                    node.dataState[blank - 1] == CONSTANTS.LEFT_ARROW):
                node.modify(blank, blank - 2)
            # Slide: right-facing frog moves one position forward into the empty slot
            else:
                node.modify(blank, blank - 1)
        else:
            # Jump: left-facing frog leaps two positions over the opposite-facing frog in between
            if (blank <= size - 3 and
                    node.dataState[blank + 2] == CONSTANTS.LEFT_ARROW and
                    node.dataState[blank + 1] == CONSTANTS.RIGHT_ARROW):
                node.modify(blank, blank + 2)
            # Slide: left-facing frog moves one position forward into the empty slot
            else:
                node.modify(blank, blank + 1)

    def solve(self):
        yield str(self.stateNode)
        for (direction, count) in self.generateMoveGroups():
            for _ in range(count):
                self.applyNextMove(direction)
                yield str(self.stateNode)


def solveTask() -> None:
    N = int(sys.stdin.readline().strip())

    startStateNode = BUILD.buildNode(N, StateNodeType.START)

    solver = RuleBasedSolver(startStateNode, N)

    out = sys.stdout.write
    for snapshot in solver.solve():
        out(snapshot + '\n')


if __name__ == "__main__":
    solveTask()
