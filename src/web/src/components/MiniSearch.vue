<script setup>
import router from '@/router'
import {onMounted, ref} from "vue";

const props = defineProps({
    initContent: String
});

const searchContent = ref("");

function search(event) {
    if (searchContent.value.length === 0)
        return;

    const query = event.target.value;
    router.push({
        path: "/s",
        query: {
            query: query
        }
    });
}

onMounted(() => {
    if (props.initContent)
        searchContent.value = props.initContent;
});
</script>

<template>
    <div class="banner h-[100px] w-full flex flex-row justify-between items-center px-9 bg-white have-bottom-line">
        <router-link to="/">
            <div class="flex flex-row items-center justify-start sm:justify-center">
                <img src="/logo.png" class="object-contain w-[48px] h-[48px]">
                <h1 class="text-3xl font-light">
                    ScholarInsight
                </h1>
            </div>
        </router-link>


        <input
            class="miniSearch"
            type="text" placeholder="输入搜索内容" @keydown.enter="search" v-model="searchContent"/>
    </div>
</template>
