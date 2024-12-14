<script setup>
import {computed, onMounted, ref, watch} from "vue";
import {backend} from "@/utils/api.js";
import EntryCard from "@/components/EntryCard.vue";

const props = defineProps({
    entryList: Array,
    fullMatchList: Array | null
});

const pageKeyChunk = ref([]);

const currentPage = ref(1);
const maxPage = ref(1);
const entryPerPage = 15;
const pageInputValue = ref(1); // Binding of the input component

const pageRender = ref([]);

const isPageLoaded = ref(false);
const isPaginated = ref(false);

function executeWorker(entryList, entryPerPage) {
    return new Promise((resolve, reject) => {
        const worker = new Worker("/pageWorker.js");
        worker.postMessage([JSON.stringify(entryList), entryPerPage]);
        worker.onmessage = e => {
            resolve(e.data);
        };
        worker.onerror = reject;
    });
}

onMounted(() => {
    maxPage.value = Math.ceil(props.entryList.length / entryPerPage);
    if (currentPage.value > maxPage.value)
        currentPage.value = maxPage.value;

    if (maxPage.value < 10000) {
        console.log("Main thread working...");
        slicePages();

        queryPage(0);
        isPaginated.value = true;
    } else {
        console.log("Using webworker...");
        pageKeyChunk.value.push(props.entryList.slice(0, entryPerPage));
        queryPage(0);

        setTimeout(() => {
            executeWorker(props.entryList, entryPerPage)
                .then(data => {
                    pageKeyChunk.value = data;
                    isPaginated.value = true;
                })
                .catch(error => {
                    console.error('Error:', error);
                });
        }, 0);
        console.log("Promise sent");
    }
});

function slicePages() {
    for (let i = 0; i < props.entryList.length; i += entryPerPage) {
        pageKeyChunk.value.push(props.entryList.slice(i, i + entryPerPage));
    }
}

async function queryPage(index) {
    let target = [];
    if (haveFullMatch.value) {
        if (index == 0)
            target = props.fullMatchList;
        else
            target = pageKeyChunk.value[index - 1];
    } else {
        target = pageKeyChunk.value[index];
    }

    let data = await backend.post("hexkeylist2entry", {
        "keylist": target
    });
    pageRender.value = data.data.result;
    isPageLoaded.value = true;
}

watch(currentPage, (index) => {
    isPageLoaded.value = false;
    queryPage(index - 1);
});

function changePage(page) {
    // check if page is a number
    if (isNaN(page)) {
        page = parseInt(page);
        if (isNaN(page)) {
            pageInputValue.value = currentPage.value;
            return;
        }
    }

    page = Math.max(1, Math.min(maxPage.value, page));
    currentPage.value = page;
    pageInputValue.value = page;
}

function increment() {
    currentPage.value = Math.min(currentPage.value + 1, maxPage.value);
    pageInputValue.value = currentPage.value;
}

function decrement() {
    currentPage.value = Math.max(currentPage.value - 1, 1);
    pageInputValue.value = currentPage.value;
}

const haveFullMatch = computed(() => {
    if (props.fullMatchList === null || props.fullMatchList === undefined)
        return false;
    return props.fullMatchList.length > 0;
});
</script>

<template>
    <Transition>
        <div class="py-6 have-bottom-line" v-if="isPageLoaded">
            <p class="text-c-grey py-4" v-if="haveFullMatch && currentPage === 1">完全匹配结果</p>
            <slot name="entry-card" v-for="e in pageRender" :entry="e">
                <EntryCard :entry="e">
                </EntryCard>
            </slot>
        </div>
        <div class="py-6 have-bottom-line" v-else>
            <div class="is-loading h-[95px]" v-for="i in pageKeyChunk[currentPage - 1]">
            </div>
        </div>
    </Transition>

    <div class="flex flex-row justify-center items-center gap-4 my-8" v-if="maxPage !== 1 && isPaginated">
        <button @click="decrement" class="Button mr-2" v-if="currentPage !== 1">上一页</button>
        <input class="nav-input" type="text" @keydown.enter="changePage(pageInputValue)" v-model="pageInputValue">
        <span> / {{ maxPage }}</span>
        <button @click="changePage(pageInputValue)" class="Button primary">跳转</button>
        <button @click="increment" class="Button ml-2" v-if="currentPage !== maxPage">下一页</button>
    </div>
    <div class="is-loading w-full my-8 h-[60px]" v-if="!isPaginated">
    </div>
</template>

<style>
.v-enter-active,
.v-leave-active {
    transition: opacity 0.2s ease;
}

.v-enter-from,
.v-leave-to {
    opacity: 0;
}
</style>