<template>
    <div class="w-full">
        <h1> Analytics - {{ props.func }}</h1>
        <div class="card">
            <DataTable :value="tableData" tableStyle="min-width: 50rem" paginator :rows="10"
                :rowsPerPageOptions="[10, 20, 50]">
                <Column v-for="col of columns" :key="col.field" :field="col.field" :header="col.header"></Column>
            </DataTable>
        </div>
    </div>
</template>

<script lang="ts" setup>
import { ref, watch } from 'vue';
import { DataTable, Column } from 'primevue';
import { defineProps } from 'vue';
import { onMounted } from 'vue';
import { useToast } from 'primevue';
import { useUserStore } from '@/stores/userStore';
import router from '@/router';


const props = defineProps({
    func: {
        type: String,
        required: true
    }
});

const toast = useToast();
const userStore = useUserStore();

watch(() => userStore.role, (newRole) => {
    if (newRole !== "ADMIN") {
        toast.add({ severity: 'error', summary: 'Permission Denied', detail: 'You do not have permission to access this page', life: 3000 });
        router.push('/');
    }
});

const tableData = ref();
const columns = ref([
    { field: '', header: '' },
]);

const getCustomers = () => {
    fetch('/api/user')
        .then(response => response.json())
        .then(data => {
            console.log(data);
            if (data.code === 0) {
                tableData.value = data.data.filter(customer => customer.role === 'CUSTOMER');
                columns.value = [
                    { field: 'id', header: 'ID' },
                    { field: 'name', header: 'Username' },
                    { field: 'email', header: 'Email' },
                    // { field: 'role', header: 'Role' },
                ];
            } else {
                console.error('Error fetching customers:', data);
                toast.add({ severity: 'error', summary: 'Error fetching customers', detail: data.message, life: 3000 });
            }
        })
        .catch(error => {
            console.error('Error fetching customers:', error);
            toast.add({ severity: 'error', summary: 'Error fetching customers', detail: error, life: 3000 });
        });
};

const getAna = (action: string) => {
    fetch(`/api/analysis/log?action=${action}`)
        .then(response => response.json())
        .then(data => {
            console.log(data);
            if (data.code === 0) {
                // tableData.value = data.data;
                tableData.value = data.data.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

                columns.value = [
                    { field: 'id', header: 'Record ID' },
                    { field: 'detail', header: 'Detail' },
                    { field: 'timestamp', header: 'Timestamp' },
                ];

            } else {
                console.error('Error fetching visits:', data);
                toast.add({ severity: 'error', summary: 'Error fetching visits', detail: data.message, life: 3000 });
            }
        })
        .catch(error => {
            console.error('Error fetching visits:', error);
            toast.add({ severity: 'error', summary: 'Error fetching visits', detail: error, life: 3000 });
        });
};



const updatePage = (newFunc) => {
    if (newFunc === 'customers') {
        getCustomers();
    } else if (newFunc === 'visits') {
        getAna("VISIT");
    } else if (newFunc === 'sales') {
        getAna("PURCHASE");
    }else {
        toast.add({ severity: 'error', summary: 'Invalid Function', detail: 'Invalid function name', life: 3000 });
        router.push('/');
    }
};

watch(() => props.func, updatePage);
onMounted(() => {
    updatePage(props.func);
});

</script>
