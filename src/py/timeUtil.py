import time
import tqdm


class QueryTimeHelper:
    def __init__(self) -> None:
        self.current = 0
        self.prev = 0

    def start(self) -> None:
        self.prev = time.time()

    def time(self) -> float:
        self.current = time.time()
        res = self.current - self.prev
        self.prev = self.current
        return res


class InsertTimeHelper:
    def __init__(self):
        self.total = 0
        self.tqdm = tqdm.tqdm(total=11000000)

    def __call__(self) -> None:
        self.total += 1000
        self.tqdm.update(1000)
