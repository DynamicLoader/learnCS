<script setup>
import MiniSearch from '@/components/MiniSearch.vue';
import {ref, onMounted, watch, computed} from 'vue';
import {useRoute} from 'vue-router';
import {backend} from '@/utils/api';

import AttrDisplay from "@/components/AttrDisplay.vue";

const result = ref({});
const route = useRoute();
const entryName = route.query.entry;
const loaded = ref(false);

function query() {
    console.log(route.query);

    if (!route.query.entry)
        return;

    backend.post("/fullentry", {
        query: route.query.entry
    }).then(data => {
        data = data.data[0];
        console.log(data);

        result.value = {};
        result.value.title = data.props.title[0];
        delete data.props.title;

        result.value.props = Object.entries(data.props);
        result.value.attrs = Object.entries(data.attrs);

        console.log(result.value);
        loaded.value = true;
    });
}

watch(route, () => {
    loaded.value = false;
    query();
}, {immediate: true});
</script>

<template>
    <div>
        <MiniSearch class="fixed top-0 left-0 w-full"/>
        <div class="h-full pt-[100px]">
            <div class="container overflow-hidden" v-if="loaded">
                <h2 class="text-2xl py-8 have-bar have-bottom-line leading-loose">
                    {{ result ? result.title : "" }}
                </h2>

                <div class="flex flex-col md:flex-row justify-between pb-24 pt-4">
                    <div class="md:w-1/2">
                        <h3 class="text-lg font-bold pt-4">
                            文章信息
                        </h3>
                        <AttrDisplay v-for="p in (result ? result.props : [])" is-list :list-data="p[1]" :tag="p[0]"/>
                    </div>

                    <div class="md:w-1/2">
                        <h3 class="text-lg font-bold pt-4">
                            其他标志
                        </h3>
                        <AttrDisplay v-for="a in (result ? result.props : [])" is-string :str-data="a[1][0]"
                                     :tag="a[0]"/>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>