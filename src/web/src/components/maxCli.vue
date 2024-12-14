<script setup>
import { defineProps } from 'vue'
const props = defineProps({
    MaxCli: Object
})
const chineseNumbers = ['零', '一', '二', '三', '四', '五', '六', '七', '八', '九', '十']
</script>

<template>
    <div>
        <ol class="list-decimal ml-5 space-y-2">
            <li v-for="(item, index) in MaxCli"
                class="text-gray-700 text-lg text-center font-semibold bg-gray-100 p-3 my-1 rounded shadow flex items-center justify-center"
                :key="item">
                <span>第{{ chineseNumbers[index] }}阶:</span> <span class="ml-2">{{ item }}</span>
            </li>
        </ol>
    </div>
</template>

<style scoped>
/* Your component styles go here */
</style>