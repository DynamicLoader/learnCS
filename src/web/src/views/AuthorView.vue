<script setup>
import MiniSearch from '@/components/MiniSearch.vue';
import AuthorInformation from '@/components/AuthorInformation.vue';
import EntryCard from '@/components/EntryCard.vue';
import PageDisplay from "@/components/PageDisplay.vue";
import CoauthorCard from '@/components/CoauthorCard.vue';
import { ref, onMounted, watch, provide } from 'vue';
import { useRoute } from 'vue-router';
import { backend } from '@/utils/api';

const currentTab = ref('PageDisplay');
const route = useRoute();
// const authorName = route.query.author;
const result = ref({});
const Top = ref({});
const ranking = ref("0");

const isLoaded = ref(false);
// let callback = null;
// function setCallback(cb) {
//     callback = cb;
// }

// provide("setCallback", setCallback);

async function query() {
    console.log(route.query);

    if (!route.query.author)
        return;

    await backend.post("query", {
        query: route.query.author
    }).then(data => {
        console.log(data)
        result.value = data.data;

        // if (callback)
        //     callback();
    });

    await backend.post("/feature", {
        query: route.query.query

    }).then(data => {
        console.log(data);
        Top.value = data.data;
        ranking.value = foundRank();
        console.log(ranking.value);
    });

    isLoaded.value = true;
}

function foundRank() {
    let i = 0;
    for (i = 0; i < Top.value.topauthors.length; i++) {
        if (Top.value.topauthors[i] == result.value.author.name) {
            return i + 1;
        }
    }
    return i + 1;
}


watch(route, () => {
    query();
}, { immediate: true });

</script>

<template>
    <div>
        <MiniSearch class="fixed top-0 left-0 w-full" />
        <div class="h-full pt-[100px]" v-if="isLoaded">
            <div class="container overflow-hidden">
                <div v-if="result">
                    <AuthorInformation :author="result.author" :ranking="ranking" />
                    <div>
                        <div class="flex have-bottom-line">
                            <button class="py-2 px-4"
                                :class="{ 'border-b-2 border-blue-500': currentTab === 'PageDisplay' }"
                                @click="currentTab = 'PageDisplay'">作品</button>
                            <button class="py-2 px-4"
                                :class="{ 'border-b-2 border-blue-500': currentTab === 'CoauthorCard' }"
                                @click="currentTab = 'CoauthorCard'">合作者</button>
                        </div>
                        <div v-if="currentTab === 'PageDisplay'">
                            <PageDisplay :entry-list="result.author.work">
                                <EntryCard :entry="entry" v-for="entry in result.author.work" :key="entry.pkey" />
                            </PageDisplay>
                        </div>
                    </div>
                </div>

                <div v-if="currentTab === 'CoauthorCard'">
                    <CoauthorCard :author="result.author" v-show="currentTab === 'CoauthorCard'" class="overflow-hidden" />
                </div>
            </div>

        </div>
    </div>
</template>


<style scoped>
button {
    transition: all .3s ease;
}

button:hover {
    background-color: #f3f3f3;
}
</style>