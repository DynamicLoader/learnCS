import { createRouter, createWebHistory } from 'vue-router'
const HomeView = () => import('../views/HomeView.vue')
const SearchView = () => import('@/views/SearchView.vue')
const AuthorView = () => import('@/views/AuthorView.vue')
const EntryView = () => import('@/views/EntryView.vue')
const StatView = () => import('@/views/StatView.vue')
const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView
    },
    {
      path: "/s",
      name: "search",
      component: SearchView
    },
    {
      path: "/author",
      name: "author information",
      component: AuthorView
    },
    {
      path: "/fullentry",
      name: "entry information",
      component: EntryView
    },
    {
      path: "/stats",
      name: "stats",
      component: StatView
    },
    {
      path: '/:catchAll(.*)',
      name: 'not-found',
      component: () => import('@/views/404.vue')
    }


  ]
})

export default router
