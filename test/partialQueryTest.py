import sys

sys.path.append("src/py/")
from serviceWrapper import ServiceWrapper
import time

s = ServiceWrapper()

resultList = []
for key in s.queryByKeywordRaw("adaptive method"):
    resultList.append(key.toString())

chunkedResultList = [resultList[i : i + 15] for i in range(0, len(resultList), 15)]

start_time = time.time()
for keylist in chunkedResultList:
    s.hexString2result(keylist)

end_time = time.time()
execution_time = (end_time - start_time) * 1000
print(f"Execution time: {execution_time / len(chunkedResultList)} ms")
