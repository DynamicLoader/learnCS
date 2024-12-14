<script setup>
import {ref, onMounted, watch} from 'vue';
import {useRoute} from 'vue-router';
import {backend} from '@/utils/api';

import MiniSearch from '@/components/MiniSearch.vue'
import AuthorCard from '@/components/AuthorCard.vue';
import PageDisplay from "@/components/PageDisplay.vue";
import PageDisplayRaw from "@/components/PageDisplayRaw.vue";

const loaded = ref(false);

const route = useRoute();
const result = ref({});

function query() {
    console.log(route.query);

    if (!route.query.query) {
        result.value = {
            keyword: [],
        };
        loaded.value = true;
        return;
    }

    // 为什么不用axios? 因为response transform会花费大量时间.
    fetch('/api/query', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            query: route.query.query
        }),
    })
        .then(response => {
                return response.json()
            }
        )
        .then(data => {
            console.log(data);
            result.value = data;
            loaded.value = true;
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

watch(route, () => {
    loaded.value = false;
    query();
}, {immediate: true});

</script>

<template>
    <div>
        <MiniSearch class="fixed top-0 left-0 w-full" :init-content="$route.query.query"/>

        <div class="pt-[100px]">
            <div class="container overflow-hidden" v-if="loaded">
                <div v-if="result.author">
                    <AuthorCard :author="result.author"/>
                    <PageDisplay :entryList="result.author.work"/>
                </div>

                <div v-else-if="result.keyword.length > 0">
                    <div class="have-bottom-line px-6 py-4 text-c-grey">
                        共有 {{ result.keyword.length }} 个匹配条目
                    </div>
                    <PageDisplay :entryList="result.keyword" :full-match-list="result.fullmatch"/>
                </div>

                <div v-else class="text-c-grey text-2xl py-8 text-center have-bottom-line">
                    没有结果
                </div>
            </div>

            <div class="container overflow-clip pt-4" v-else>
                <div class="is-loading fake-search-loading">
                </div>
            </div>
        </div>
    </div>
</template>

<style scoped>
.fake-search-loading {
    height: calc(min(800px, 100vh - 100px - 5rem))
}
</style>