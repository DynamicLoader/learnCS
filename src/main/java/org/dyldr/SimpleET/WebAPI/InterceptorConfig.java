package org.dyldr.SimpleET.WebAPI;

import org.dyldr.SimpleET.WebAPI.JWTInterceptor;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.config.annotation.InterceptorRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;


@Configuration
public class InterceptorConfig implements WebMvcConfigurer {

    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        // 注册声明的拦截器
        registry.addInterceptor(new JWTInterceptor())
                .addPathPatterns("/api/**")
                .excludePathPatterns("/api/auth/*") // Allow auth
                .excludePathPatterns("/payment/callback/*"); // Allow callback, we will check the token in the callback
    }
}