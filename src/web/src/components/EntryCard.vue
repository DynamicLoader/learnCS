<script setup>
import {computed} from "vue";

const props = defineProps({
    entry: Object
})

const authorDisplay = computed(() => {
    if (!props.entry.author)
        return [];

    let res = [];
    let len = Math.min(props.entry.author.length, 3);
    for (let i = 0; i < len; i++)
        res.push(props.entry.author[i]);

    return res;
});
</script>

<template>
    <div class="my-1 mx-3 py-4 px-3 flex-flex-col w-full hover:bg-gray-100 rounded-lg">
        <router-link :to="{path: '/fullentry', query: {entry: props.entry.pkey}}">
            <h3 class="text-lg mx-2">
                {{ props.entry.title[0] }}
            </h3>
        </router-link>
        <div class="flex flex-row items-center">
            <router-link v-for="a in authorDisplay" :to="{path: '/author', query: {author: a}}">
                <p class="block mx-2 my-1 text-c-light">
                    {{ a }}
                </p>
            </router-link>
            <span v-if="props.entry.author.length > 3" class="text-c-light">
                等 {{ props.entry.author.length }} 个作者
            </span>

            <p class="text-c-grey ml-4 have-bar is-secondary">
                {{ props.entry.year[0] }}
            </p>
        </div>
    </div>
</template>