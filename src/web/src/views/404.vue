<script setup>
import MiniSearch from '@/components/MiniSearch.vue';
import PageDisplay from '@/components/PageDisplay.vue';
import router from '@/router';
import { computed, ref } from 'vue';

const randomJump = computed(() => {
    const randomNumber = Math.floor(Math.random() * 10000000) + 1;
    return randomNumber.toString(16);
});

</script>

<template>
    <MiniSearch></MiniSearch>

    <div class="pt-100 h-full w-full">
        <div class="container py-8 flex flex-col">
            <h1 class="text-center text-[80px] font-bold text-c-light py-8">
                404
            </h1>
            <p class="text-center text-lg text-c-light italic">
                是不存在的页面呢。不妨看看这篇...
            </p>

            <PageDisplay :entryList="[randomJump]">
            </PageDisplay>

            <div class="flex flex-row mt-8 items-center justify-center">
                <button class="Button primary" @click="router.push('/')">
                    返回主页
                </button>
            </div>
        </div>
    </div>
</template>