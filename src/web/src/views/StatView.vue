<script setup>
import Minisearch from "@/components/MiniSearch.vue";
import top100list from "@/components/top100list.vue";
import maxCli from "@/components/maxCli.vue";
import { ref, onMounted, watch } from "vue";
import { useRoute } from "vue-router";
import { backend } from "@/utils/api";
import topics from "@/components/topics.vue";

const route = useRoute();
const result = ref({});
function query() {
    backend
        .post("/feature", {
            query: route.query.query,
        })
        .then((data) => {
            console.log(data);
            result.value = data.data;
        });
}
//
onMounted(query);
</script>

<template>
    <div>
        <Minisearch class="fixed top-0 left-0 w-full z-[9999]" />
        <div class="h-full pt-[100px] w-full">
            <div
                class="flex flex-col justify-center space-y-8 container items-center"
            >
                <div>
                    <p class="text-center rainbow py-8">
                        排行榜
                    </p>
                    <div
                        class="grid grid-cols-1 md:grid-cols-2 gap-8"
                    >
                        <div
                            class="bg-white shadow-lg overflow-hidden sm:rounded-lg"
                        >
                            <p
                                class="text-2xl leading-6 font-semibold text-gray-900 py-4 text-center"
                            >
                                Top Authors
                            </p>
                            <top100list
                                :top100="result.topauthors"
                                :ty="'author'"
                            />
                        </div>
                        <div
                            class="bg-white shadow-lg overflow-hidden sm:rounded-lg"
                        >
                            <p
                                class="text-2xl leading-6 font-semibold text-gray-900 py-4 text-center"
                            >
                                Top Keywords
                            </p>
                            <top100list
                                :top100="result.topkeywords"
                                :ty="'keyword'"
                            />
                        </div>
                    </div>
                </div>
                <div>
                    <p class="text-center rainbow py-8">
                        年度热点
                    </p>
                    <topics
                        :topyearkeywords="
                            result.topyearkeywords
                        "
                    />
                </div>
                <div>
                    <p class="text-center rainbow py-4">
                        最大团统计
                    </p>
                    <maxCli :MaxCli="result.MaxCli" class="py-4"/>
                </div>
            </div>
        </div>
    </div>
</template>

<style scoped>
/* Your component-specific styles go here */
</style>
