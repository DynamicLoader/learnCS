<script setup>
import {ref} from 'vue';
import router from '@/router';

const searchContent = ref("");

function handleQuery() {
    if (searchContent.value.length < 1)
        return;

    router.push({
        path: "/s",
        query: {
            query: searchContent.value
        }
    });
}

function feelLucky() {
    const randomNumber = Math.floor(Math.random() * 10000000) + 1;
    const hexNumber = randomNumber.toString(16);

    router.push({
        path: "/fullentry",
        query: {
            entry: hexNumber
        }
    });
}

</script>

<template>
    <div class="w-full h-[100vh]">
        <img src="/bg.png" class="h-full w-full object-cover absolute left-0 top-0 z-[-1]">
        <div class="container flex flex-col h-full justify-center z-[100] home-container shadow-lg">
            <div class="flex flex-row mx-12 items-center justify-start">
                <img src="/logo.png" class="object-contain w-[72px] h-[72px]">
                <h1 class="text-4xl font-light">
                    ScholarInsight
                </h1>
            </div>

            <input type="text" class="search self-center my-4" v-model="searchContent" @keydown.enter="handleQuery()">

            <div class="flex flex-row gap-4 self-center mt-2">
                <button class="Button primary" @click="handleQuery()">搜索</button>
                <button class="Button" @click="feelLucky()">手气不错</button>
                <router-link to="/stats">
                    <button class="Button">统计发现</button>
                </router-link>
            </div>

            <p class="text-c-grey text-sm font-light italic self-center pt-12">"站在先辈的肩膀上"</p>

        </div>
    </div>
</template>

<style scoped>
.home-container {
    background: rgba(255, 255, 255, 0.18);
    backdrop-filter: blur(60px);
}
</style>