<script setup>
import {onMounted, ref, watch} from "vue";
import EntryCard from "@/components/EntryCard.vue";

const props = defineProps({
    entryList: Array
});

const pages = ref([]);

const currentPage = ref(1);
const maxPage = ref(1);

const entryPerPage = 15;
const pagevalue = ref(1);

onMounted(() => {
    maxPage.value = Math.ceil(props.entryList.length / entryPerPage);

    if (currentPage.value > maxPage.value)
        currentPage.value = maxPage.value;
    for (let i = 0; i < props.entryList.length; i += entryPerPage) {
        pages.value.push(props.entryList.slice(i, i + entryPerPage));
    }
});

function changePage(page) {
    // check if page is a number
    if (isNaN(page)) {
        page = parseInt(page);
        if (isNaN(page)) {
            pagevalue.value = currentPage.value;
            return;
        }
    }

    page = Math.max(1, Math.min(maxPage.value, page));
    currentPage.value = page;
    pagevalue.value = page;
}

function increment() {
    currentPage.value = Math.min(currentPage.value + 1, maxPage.value);
    pagevalue.value = currentPage.value;
}

function decrement() {
    currentPage.value = Math.max(currentPage.value - 1, 1);
    pagevalue.value = currentPage.value;
}
</script>

<template>
    <div class="py-6 have-bottom-line">
        <EntryCard v-for="e in pages[currentPage - 1]" :entry="e">
        </EntryCard>
    </div>

    <div class="flex flex-row justify-center items-center gap-4 my-8" v-if="maxPage !== 1">
        <button @click="decrement" class="Button mr-2" v-if="currentPage !== 1">上一页</button>
        <input class="nav-input" type="text" @keydown.enter="changePage(pagevalue)" v-model="pagevalue">
        <span> / {{ maxPage }}</span>
        <button @click="changePage(pagevalue)" class="Button primary">跳转</button>
        <button @click="increment" class="Button ml-2" v-if="currentPage !== maxPage">下一页</button>
    </div>
</template>