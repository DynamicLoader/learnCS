<script setup>
const props = defineProps({
    author: Object
});
</script>

<template>
    <div class="border-solid border-gray-300 rounded-md border-[1px] p-6 my-3">
        <p class="text-sm text-gray-400">作者结果</p>
        <router-link :to="{path: '/author', query: {query: props.author.name}}">
            <p class="text-xl font-bold my-4">{{ props.author.name }}</p>
        </router-link>
        <p class="text-gray-500">{{ props.author.workcount }} 篇作品</p>
    </div>
</template>