<script setup>
const props = defineProps({
    isList: Boolean,
    isString: Boolean,
    listData: Array,
    strData: String,
    tag: String
});

function UpperFirst(s) {
    if (!s || typeof s !== 'string') {
        return '';
    }

    if (s.length <= 3)
        return s.toUpperCase();

    return s.charAt(0).toUpperCase() + s.slice(1);
}
</script>

<template>
    <div class="my-2 px-2 w-full">
        <div v-if="props.isList">
            <p class="text-lg pt-4 have-bar is-secondary">
                {{ UpperFirst(props.tag) }}
            </p>
            <ul v-if="props.tag === 'author'">
                <router-link v-for="i in props.listData" :to="{path: '/author', query: {author: i}}">
                    <li class="my-2 mx-2 text-c-grey"> {{ i }}</li>
                </router-link>
            </ul>
            <ul v-else-if="props.tag === 'ee'">
                <li v-for="i in props.listData" class="my-2 px-3 text-c-grey">
                    <a :href="i" class="doilink">
                        {{ i }}
                    </a>
                </li>
            </ul>
            <ul v-else>
                <li v-for="i in props.listData" class="my-2 px-3 text-c-grey"> {{ i }}</li>
            </ul>
        </div>

        <div v-if="props.isString">
            <p class="have-bar is-secondary text-c-grey pt-2">
                <span class="text-black">{{ UpperFirst(props.tag) }}</span>
                {{ props.strData }}
            </p>
        </div>
    </div>
</template>