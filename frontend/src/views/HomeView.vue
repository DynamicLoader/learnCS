<script setup lang="ts">
import { Button, FloatLabel, InputGroup, InputText, InputGroupAddon, RadioButton, Checkbox, SpeedDial, Toast } from 'primevue';
import ItemList from '../components/ItemList.vue';
import ProductItem from '../components/ProductItem.vue';
import { ref } from 'vue';
import { useRoute, useRouter } from 'vue-router';
import { computed } from 'vue';
import { onMounted } from 'vue';
import { useUserStore } from '@/stores/userStore';
import { useToast } from 'primevue';

const products = ref([]);
const route = useRoute();
const router = useRouter();
const category = ref(route.query.category || '');
const sellername = ref(route.query.sellername || '');
const name = ref(route.query.name || '');
const minPrice = ref(route.query.minPrice || 0);
const maxPrice = ref(route.query.maxPrice || 1000000);
const userStore = useUserStore();
const toast = useToast();

const fetchProducts = async () => {
  const queryParams = new URLSearchParams();
  if (category.value !== '') {
    queryParams.append('category', category.value.toString());
  }
  if (sellername.value !== '') {
    queryParams.append('sellername', sellername.value.toString());
  }
  if (name.value !== '') {
    queryParams.append('name', name.value.toString());
  }
  if (minPrice.value !== 0) {
    queryParams.append('minPrice', minPrice.value.toString());
  }
  if (maxPrice.value !== 1000000) {
    queryParams.append('maxPrice', maxPrice.value.toString());
  }
  const queryString = queryParams.toString();

  try {
    const response = await fetch(`/api/product?${queryString}`);
    const data = await response.json();
    // products.value = data;    
    if (data.code === 401) {
      router.push('/auth/login');
      return;
    }
    if (data.code === 0) {
      products.value = data.data.map(product => ({
        pid: product.pid,
        link: `/product/${product.pid}`,
        primaryLink: `/buy/single/${product.pid}`,
        secondaryLink: `/cart/add/${product.pid}`,
        name: product.name,
        // category: product.category,
        image: product.image,
        inventoryStatus: product.stock === 0 ? 'OUTOFSTOCK' : 'INSTOCK',
        severity: product.stock === 0 ? 'danger' : 'success',
        totalSold: product.totalSold,
        // rating: 100,
        price: "CNY " + product.price.toFixed(2),
      }));
    }
    console.log(data);
    // console.log(products.value);
    
  } catch (error) {
    console.error('Error fetching products:', error);
    toast.add({ severity: 'error', summary: 'Error fetching products', life: 3000 });
  }
};

import { watch } from 'vue';

const categoryChecked = ref(false);
const minPriceChecked = ref(false);
const maxPriceChecked = ref(false);

const queryName = ref('');
const queryCategory = ref('');
const queryMinPrice = ref('');
const queryMaxPrice = ref('');


const doUpdateAndQuery = (newQuery) => {
  category.value = newQuery.category || '';
  sellername.value = newQuery.sellername || '';
  name.value = newQuery.name || '';
  minPrice.value = newQuery.minPrice || 0;
  maxPrice.value = newQuery.maxPrice || 1000000;

  queryName.value = newQuery.name?.toString() || '';
  queryCategory.value = newQuery.category?.toString() || '';
  queryMinPrice.value = newQuery.minPrice?.toString() || '';
  queryMaxPrice.value = newQuery.maxPrice?.toString() || '';

  fetchProducts();
}

watch(
  () => route.query,
  doUpdateAndQuery
);

onMounted(() => {
  const newQuery = route.query;
  doUpdateAndQuery(newQuery);
});

function navigateToLink(link: string) {
  router.push(link);
}


function updateParam() {
  const keyword = queryName.value;
  const categoryValue = categoryChecked.value ? queryCategory.value : '';
  const minPriceValue = minPriceChecked.value ? parseFloat(queryMinPrice.value) : 0;
  const maxPriceValue = maxPriceChecked.value ? parseFloat(queryMaxPrice.value) : 1000000;

  category.value = categoryValue;
  minPrice.value = minPriceValue;
  maxPrice.value = maxPriceValue;
  name.value = keyword;

  type QueryParams = {
    category?: string;
    sellername?: string;
    name?: string;
    minPrice?: string;
    maxPrice?: string;
  };

  const queryParams: QueryParams = {};
  if (category.value !== '') {
    queryParams.category = category.value.toString();
  }
  if (sellername.value !== '') {
    queryParams.sellername = sellername.value.toString();
  }
  if (name.value !== '') {
    queryParams.name = name.value.toString();
  }
  if (minPrice.value !== 0) {
    queryParams.minPrice = minPrice.value.toString();
  }
  if (maxPrice.value !== 1000000) {
    queryParams.maxPrice = maxPrice.value.toString();
  }
  // console.log(queryParams);
  // const queryString = Array.from(queryParams).map(([key, value]) => `${key}=${value}`).join('&');
  // router.push({ path: '/?' + queryParams });
  router.replace({ path: '/', query: queryParams });
}

</script>


<template>
  <div class="card flex flex-col w-full">

    <InputGroup>
      <InputText id="keyword" v-model="queryName" placeholder="Keyword" />
      <Button label="Search" @click="updateParam()" />
      <Button label="Create New" outlined v-if="userStore.role !== 'CUSTOMER'" @click="navigateToLink('/product/new')" />
    </InputGroup>

  </div>
  <br>
  <div class="card flex flex-col w-full md:flex-row gap-4">

    <InputGroup>
      <InputGroupAddon>
        <Checkbox v-model="categoryChecked" :binary="true" />
      </InputGroupAddon>
      <FloatLabel variant="on">
        <InputText id="category" v-model="queryCategory" />
        <label for="category">Category</label>
      </FloatLabel>
    </InputGroup>

    <InputGroup>
      <InputGroupAddon>
        <Checkbox v-model="minPriceChecked" :binary="true" />
      </InputGroupAddon>
      <FloatLabel variant="on">
        <InputText id="minp" v-model="queryMinPrice" />
        <label for="minp">Min Price (0) </label>
      </FloatLabel>

    </InputGroup>

    <InputGroup>
      <InputGroupAddon>
        <Checkbox v-model="maxPriceChecked" :binary="true" />
      </InputGroupAddon>
      <FloatLabel variant="on">
        <InputText id="maxp" v-model="queryMaxPrice" />
        <label for="maxp">Max Price (1000000)</label>
      </FloatLabel>

    </InputGroup>
  </div>
  <ItemList :items="products" :itemComponent="ProductItem" />
</template>
