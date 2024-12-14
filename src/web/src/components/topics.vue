<script setup>
import { defineProps, computed } from "vue";
const props = defineProps({
    topyearkeywords: Object,
});

function isNumeric(str) {
    if (typeof str !== "string") return false;
    return !isNaN(str) && !isNaN(parseFloat(str));
}
// 计算属性，将 topyearkeywords 转换为 "年份 -> 热点词" 的结构
</script>

<template>
    <div v-for="s in props.topyearkeywords">
        <p class="text-c-light text-3xl font-light pl-8 pb-2 pt-6" v-if="isNumeric(s)">
            {{ s }}
        </p>
        <div class="text-gray-700 text-lg text-center font-semibold bg-gray-100 p-2 rounded shadow" v-else>
            {{ s }}
        </div>
    </div>
</template>
