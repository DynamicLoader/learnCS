import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView,
      meta: {}
    },
    {
      path: '/about',
      name: 'about',
      // route level code-splitting
      // this generates a separate chunk (About.[hash].js) for this route
      // which is lazy-loaded when the route is visited.
      component: () => import('../views/AboutView.vue'),
      meta: {}
    },
    {
      path: '/auth',
      component: () => import('../views/AuthView.vue'),
      children: [
        {
          path: 'login',
          name: 'login',
          component: () => import('../views/auth/LoginSubView.vue'),
          meta: { requiresAuth: false, name: 'Login' , notDisplayMenubar: true}
        },
        {
          path: 'register',
          name: 'register',
          component: () => import('../views/auth/RegisterSubView.vue'),
          meta: { requiresAuth: false, name: 'Register', notDisplayMenubar: true }
        },
        // {
        //   path: 'forgot-password',
        //   name: 'forgot-password',
        //   component: ForgotPassword,
        //   meta: { requiresAuth: false, name: 'ForgotPassword' }
        // }
      ]
    },
    {
      path: '/product/:id',
      name: 'product',
      component: () => import('../views/ProductView.vue'),
      meta: {}
    },
    // Cart
    {
      path: '/cart',
      name: 'cart',
      component: () => import('../views/CartView.vue'),
      meta: {}
    },
    // Buy
    {
      path: '/buy/cart',
      name: 'buy-cart',
      component: () => import('../views/BuyView.vue'),
    },
    {
      path: '/buy/single/:id',
      name: 'buy-single',
      component: () => import('../views/BuyView.vue'),
    },
    // Order
    {
      path: '/order',
      component: () => import('../views/OrderView.vue'),
    },
    {
      path: '/order/:orderid',
      component: () => import('../views/OrderDetailView.vue'),
      props: true // 允许通过 props 传递路由参数
    },
    // Analytics
    {
      path: '/analytics/:func(sales|visits|customers)',
      name: "analytics",
      component: () => import('../views/AnalyticsView.vue'),
      props: true
    }

    // Catch all
    , {
      path: '/:pathMatch(.*)*',
      name: 'not-found',
      component: () => import('../views/NotFoundView.vue'),
      meta: {}
    }

  ]
})

export default router
