import sys
from enum import IntEnum

sys.setrecursionlimit(1_000_000)


class StateNodeType(IntEnum):
    START = 0
    END = 1
    MIDDLE = 2


class StateNode:
    # Numeric IDs are used as graph keys instead of string data to avoid hashing overhead
    idMask = 0

    def __init__(self, dataState: str = "", separatorIndex: int = 0,
                 type: StateNodeType = StateNodeType.START, _default: bool = True):
        self.separatorIndex = separatorIndex
        self.dataState = dataState
        self.type = type
        self.stateNodeId = 0
        if not _default:
            self.stateNodeId = StateNode.idMask
            StateNode.idMask += 1

    @classmethod
    def create(cls, dataState: str, separatorIndex: int, type: StateNodeType) -> "StateNode":
        return cls(dataState, separatorIndex, type, _default=False)

    def modify(self, leftIndex: int, rightIndex: int) -> None:
        if leftIndex > rightIndex:
            leftIndex, rightIndex = rightIndex, leftIndex
        if rightIndex >= len(self.dataState):
            raise IndexError("Invalid position.")
        # Keeping track of where the separator ends up after the swap
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


def generateNeighbourMoves(gameStateNode: StateNode) -> list:
    resultMoves = []
    separatorIndex = gameStateNode.separatorIndex
    size = len(gameStateNode.dataState)

    # Slide: right-facing frog moves one position forward into the empty slot
    if (separatorIndex > 0 and
            gameStateNode.dataState[separatorIndex - 1] == CONSTANTS.RIGHT_ARROW):
        resultMoves.append((separatorIndex, separatorIndex - 1))

    # Jump: right-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex >= 2 and
            gameStateNode.dataState[separatorIndex - 2] == CONSTANTS.RIGHT_ARROW and
            gameStateNode.dataState[separatorIndex - 1] == CONSTANTS.LEFT_ARROW):
        resultMoves.append((separatorIndex, separatorIndex - 2))

    # Slide: left-facing frog moves one position forward into the empty slot
    if (separatorIndex < size - 1 and
            gameStateNode.dataState[separatorIndex + 1] == CONSTANTS.LEFT_ARROW):
        resultMoves.append((separatorIndex, separatorIndex + 1))

    # Jump: left-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex < size - 2 and
            gameStateNode.dataState[separatorIndex + 2] == CONSTANTS.LEFT_ARROW and
            gameStateNode.dataState[separatorIndex + 1] == CONSTANTS.RIGHT_ARROW):
        resultMoves.append((separatorIndex, separatorIndex + 2))

    return resultMoves


def dfs(visited: set, nodePath: list,
        currentNode: StateNode, goalNode: StateNode) -> bool:
    # Recording the state snapshot before exploring its children so backtracking can undo it
    currentSnapshot = "".join(currentNode.dataState)
    nodePath.append(currentSnapshot)
    if currentNode.dataState == goalNode.dataState:
        return True

    visited.add(currentSnapshot)
    for (a, b) in generateNeighbourMoves(currentNode):
        currentNode.modify(a, b)
        childSnapshot = "".join(currentNode.dataState)
        if childSnapshot not in visited:
            if dfs(visited, nodePath, currentNode, goalNode):
                return True
        # Undo the move — same swap restores the previous state
        currentNode.modify(a, b)

    nodePath.pop()
    return False


def solveTask() -> None:
    # Entering input data
    N = int(sys.stdin.readline().strip())

    # Defining start and end state of the program execution
    startStateNode = BUILD.buildNode(N, StateNodeType.START)
    endStateNode = BUILD.buildNode(N, StateNodeType.END)

    # Defining data structures used by the DFS search
    visited = set()
    nodePath = []

    # Executing the modified dfs algorithm
    if dfs(visited, nodePath, startStateNode, endStateNode):
        for snapshot in nodePath:
            print(snapshot)


if __name__ == "__main__":
    solveTask()