<script lang="ts" setup>
import { ref } from 'vue';
import 'primeicons/primeicons.css';
import Card from 'primevue/card';
import { Button, FloatLabel, InputGroup, InputText, InputGroupAddon, RadioButton, Checkbox, InputNumber, Select } from 'primevue';
import { useUserStore } from '@/stores/userStore';
import { useConfirm } from "primevue/useconfirm";
import { useToast } from "primevue/usetoast";


import { reactive } from 'vue';
import { useRoute, useRouter } from 'vue-router';

const userStore = useUserStore();
const { username, uid, role, setUser, clearUser } = userStore;

const product = reactive({

  pid: "",
  name: "",
  description: "",
  price: 0,
  stock: 0,
  category: "",
  image: "",
  seller: "",
  expressFee: 0,
  status: ""
});

const productStatus = ref(["ON_SALE", "OFF_SALE"]);
const route = useRoute();
const router = useRouter();
const productId = ref(route.params.id);
const isSeller = ref(role !== 'CUSTOMER');
// const isSeller = ref(true);
const title = ref('Product Details');
const confirm = useConfirm();
const toast = useToast();

function navigateToLink(link: string) {
  // console.log(link);
  router.push(link);
}

import { onMounted } from 'vue';
import { func } from 'superstruct';

onMounted(async () => {
  if (productId.value === "new") {
    // console.log("Userstore:",useUserStore());
    product.seller = username;
    title.value = 'New Product';
    return;
  }

  try {
    const response = await fetch(`/api/product/${productId.value}`, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json'
      }
    });
    if (response.ok) {
      const res = await response.json();
      // product.value = data.data;
      product.pid = res.data.pid;
      product.name = res.data.name;
      product.description = res.data.description;
      product.price = res.data.price.toFixed(2);
      product.stock = res.data.stock;
      product.category = res.data.category;
      product.image = res.data.image;
      product.seller = res.data.seller;
      product.expressFee = res.data.expressFee;
      product.status = res.data.status;
      console.log(res);
    } else {
      console.error('Failed to fetch product details');
    }
  } catch (error) {
    console.error('Error:', error);
  }
});

function saveProduct() {
  async function doSaveProduct(isCreate: boolean) {
    try {
      var formdata = new FormData();
      formdata.append("name", product.name);
      formdata.append("description", product.description);
      formdata.append("price", product.price.toString());
      formdata.append("stock", product.stock.toString());
      formdata.append("category", product.category);
      formdata.append("image", product.image);
      formdata.append("expressFee", product.expressFee.toString());
      formdata.append("status", product.status);

      const response = await fetch(
        isCreate ? '/api/product' : `/api/product/${productId.value}`,
        {
          method: isCreate ? 'POST' : 'PUT',
          body: formdata
        });
      if (response.ok) {
        const data = await response.json();
        console.log(data);
        if (data.code === 0) {
          // console.log('Product updated successfully');
          
          if (isCreate) {
            toast.add({ severity: 'success', summary: 'Product created', detail: `Product ${data.data} has been created`, life: 3000 });
            router.replace(`/product/${data.data}`);

          }else{
            toast.add({ severity: 'success', summary: 'Product saved', detail: 'Product saved successfully', life: 3000 });
          }
        } else {
          console.error('Failed to update product');
          toast.add({ severity: 'error', summary: 'Error while saving product', detail: data.data, life: 3000 });
        }
      } else {
        console.error('Failed to update product');
        toast.add({ severity: 'error', summary: 'Error while saving product', detail: 'An error occurred while saving the product', life: 3000 });
      }
    } catch (error) {
      console.error('Error:', error);
      toast.add({ severity: 'error', summary: 'Error', detail: 'An error occurred while saving the product', life: 3000 });
    }
  }

  doSaveProduct(productId.value === "new");
}


function deleteProduct() {
  async function doDeleteProduct() {
    try {
      const response = await fetch(`/api/product/${productId.value}`, {
        method: 'DELETE',
      });
      if (response.ok) {
        const data = await response.json();
        if (data.code === 401) {
          router.push('/auth/login');
          return;
        }
        if (data.code === 0) {
          toast.add({ severity: 'success', summary: 'Product deleted', detail: `You have deleted ${productId.value}`, life: 3000 });
          router.replace('/');
        } else {
          toast.add({ severity: 'error', summary: 'Error while deleting product', detail: data.data, life: 3000 });
        }
      }
    } catch (error) {
      toast.add({ severity: 'error', summary: 'Error', detail: 'An error occurred while deleting the product', life: 3000 });
    }
  }

  doDeleteProduct();
}

const confirmSave = () => {
  confirm.require({
    message: 'Are you sure you want to save?',
    header: 'Confirmation',
    icon: 'pi pi-exclamation-triangle',
    rejectProps: {
      label: 'Cancel',
      severity: 'secondary',
      outlined: true
    },
    acceptProps: {
      label: 'Save'
    },
    accept: () => {
      // toast.add({ severity: 'info', summary: 'Confirmed', detail: 'You have accepted', life: 3000 });
      saveProduct();
    },
  });
};

const confirmDelete = () => {
  confirm.require({
    message: 'Do you want to delete this product? It cannot be undone!!',
    header: 'Danger Zone',
    icon: 'pi pi-info-circle',
    rejectLabel: 'Cancel',
    rejectProps: {
      label: 'Cancel',
      severity: 'secondary',
      outlined: true
    },
    acceptProps: {
      label: 'Delete',
      severity: 'danger'
    },
    accept: () => {
      // toast.add({ severity: 'info', summary: 'Confirmed', detail: 'Record deleted', life: 3000 });
      deleteProduct();
    },
  });
};

const addToCart = async () => {
  var f = new FormData;
  f.append('pid', product.pid);
  f.append('count', "1");

  await fetch('/api/user/cart', {
    method: 'POST',
    body: f
  })
    .then(response => response.json())
    .then(data => {
      if (data.code === 401) {
        router.push('/auth/login');
        return;
      }

      if (data.code === 0) {
        // console.log('Item added to cart:', data);
        toast.add({ severity: 'success', summary: 'Product added to Cart', life: 3000 });
      } else {
        console.error('Error adding item to cart:', data);
        toast.add({ severity: 'error', summary: 'Error adding item to Cart' + data.data, life: 3000 });
      }
    })
    .catch(error => {
      console.error('Error adding item to cart:', error);
      toast.add({ severity: 'error', summary: 'Error adding item to Cart' + error, life: 3000 });
    });
}

</script>

<template>
  <div class="w-full p-4">
    <h1 class="text-2xl font-bold mb-4">{{ title }}</h1>
    <Card>
      <template #title v-if="isSeller">
        <InputText v-model="product.name" class="w-full" />
      </template>
      <template #title v-else>
        {{ product.name }}
      </template>
      <template #content>
        <div class="flex flex-col md:flex-row gap-4">
          <div class="flex-shrink-2">
            <img alt="Product Image" :src="product.image" class="w-32 h-32 object-cover">
          </div>
          <div class="flex-grow">
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4 pl-16">
              <div class="font-semibold">Category:</div>
              <div><template v-if="isSeller">
                  <InputText v-model="product.category" />
                </template>
                <template v-else>
                  {{ product.category }}
                </template>
              </div>
              <div class="font-semibold">Price:</div>
              <div><template v-if="isSeller">

                  <InputNumber v-model="product.price" mode="currency" currency="CNY" />
                </template>
                <template v-else>
                  CNY {{ product.price }}
                </template>
              </div>
              <div class="font-semibold">Express Fee:</div>
              <div><template v-if="isSeller">

                  <InputNumber v-model="product.expressFee" mode="currency" currency="CNY" />
                </template>
                <template v-else>
                  CNY {{ product.expressFee }}
                </template>
              </div>
              <div class="font-semibold">Stock:</div>
              <div><template v-if="isSeller">
                  <InputNumber v-model="product.stock" />
                </template>
                <template v-else>
                  {{ product.stock }}
                </template>
              </div>
              <div class="font-semibold">Seller:</div>
              <div>{{ product.seller }}</div>
              <div class="font-semibold">Status:</div>
              <template v-if="isSeller">
                <Select v-model="product.status" :options="productStatus" :placeholder="product.status"
                  class="w-full md:w-48" />
              </template>
              <template v-else>
                {{ product.status.replace("_", " ") }}
              </template>
              <div v-if="isSeller" class="font-semibold">Image URL:</div>
              <div>
                <template v-if="isSeller">
                  <InputText v-model="product.image" />
                </template>
              </div>
              <div v-if="isSeller" class="font-semibold">Description URL:</div>
              <div>
                <template v-if="isSeller">
                  <InputText v-model="product.description" />
                </template>
              </div>
            </div>

          </div>
        </div>
        <br>
      </template>
      <template #footer>
        <div class="flex flex-col md:items-end gap-8">
          <div class="flex flex-row-reverse md:flex-row gap-2 " v-if="!isSeller">
            <Button icon="pi pi-plus" outlined label="Add to Cart"
              @click="addToCart"></Button>
            <Button icon="pi pi-shopping-cart" label="Buy Now"
              @click="navigateToLink(`/buy/single/${product.pid}`)" :disabled="product.status === 'OFF_SALE' || product.stock === 0"
              class="flex-auto md:flex-initial whitespace-nowrap"></Button>
          </div>
          <div v-else class="flex flex-row-reverse md:flex-row gap-2 ">
            <Button label="Save" icon="pi pi-save" @click="confirmSave"></Button>
            <Button label="Delete" icon="pi pi-trash" class="p-button-danger" @click="confirmDelete"></Button>
          </div>
        </div>
      </template>
    </Card>

    <br>
    <div class="col-span-2">
      <img :src="product.description" alt="Description Image" class="w-full object-cover">
    </div>
  </div>
</template>