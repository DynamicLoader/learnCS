<script setup>
import {backend} from '@/utils/api';
import {ref, onMounted} from 'vue';
import axios from 'axios';
import * as G6 from '@antv/g6';
import {useRoute} from 'vue-router';

const route = useRoute();
const props = defineProps({
    author: Object
})
const authorName = ref(props.author);
const statusText = ref("");
const list = ref({})
const result = ref({})

const isLoaded = ref(false);

function query() {
    backend.post("/colab", {
        query: authorName.value.name
    }).then(data => {
        console.log("coauthor query");
        console.log(data);
        result.value = data.data;
        list.value = result.value;
        render();
    });
}


function render() {
    let edgeConfig = {size: 2, color: "#bbb"};

    const data = {
        nodes: [],
        edges: []
    };
    data.nodes.push({id: list.value.verticies[0][0], label: list.value.verticies[0][1], size: 30});
    for (let i = 1; i < list.value.verticies.length; i++)
        data.nodes.push({id: list.value.verticies[i][0], label: list.value.verticies[i][1], size: 10});
    for (let i = 0; i < list.value.edges.length; i++)
        data.edges.push({source: list.value.edges[i][0], target: list.value.edges[i][1]});

    let w = 2048;
    let h = 2048;

    const graph = new G6.Graph({
        container: 'sigma', // String | HTMLElement，必须，在 Step 1 中创建的容器 id 或容器本身
        width: w, // Number，必须，图的宽度
        height: h, // Number，必须，图的高度
        layout: {
            type: 'force2',
            workerEnabled: true,
            width: w,
            height: h,
            linkDistance: 20,
            center: [w / 2, h / 2],
            onLayoutEnd: () => {
                let graphCont = document.getElementById("theContainer");

                let isDragging = false;
                let previousX, previousY;

                graphCont.addEventListener('mousedown', (event) => {
                    isDragging = true;
                    previousX = event.clientX;
                    previousY = event.clientY;
                });

                graphCont.addEventListener('mousemove', (event) => {
                    if (isDragging) {
                        const currentX = event.clientX;
                        const currentY = event.clientY;

                        // 计算鼠标的移动距离
                        const diffX = currentX - previousX;
                        const diffY = currentY - previousY;

                        // 更新元素的位置
                        const left = graphCont.scrollLeft;
                        const top = graphCont.scrollTop;

                        graphCont.scrollLeft = left - diffX;
                        graphCont.scrollTop = top - diffY;

                        // 更新鼠标的位置
                        previousX = currentX;
                        previousY = currentY;
                    }
                });

                graphCont.addEventListener('mouseup', () => {
                    isDragging = false;
                });

                isLoaded.value = true;
                setTimeout(() => {
                        // DOM 渲染完成才可以滚动
                        graphCont.scrollTo({
                            top: h / 2 - graphCont.getBoundingClientRect().height / 2,
                            left: w / 2 - graphCont.getBoundingClientRect().width / 2,
                            behavior: "smooth"
                        });
                    }, 500
                );

            },
        }
    });
    graph.data(data); // 读取 Step 2 中的数据源到图上
    graph.render(); // 渲染图
}

onMounted(query);

</script>

<template>
    <div class="border-solid border-c-light border-[1px] rounded-xl hide-scrollbar overflow-hidden w-full h-[600px] my-8"
         id="theContainer">
        <div id="sigma" :style="isLoaded ? '' : 'display: none;'">
        </div>

        <div :style="isLoaded ? 'display: none;' : ''" class="is-loading w-full h-[600px]">
        </div>
    </div>
</template>


<style scoped>
</style>