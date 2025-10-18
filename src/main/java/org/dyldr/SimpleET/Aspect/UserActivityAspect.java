package org.dyldr.SimpleET.Aspect;

import jakarta.servlet.http.HttpServletRequest;
import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import org.dyldr.SimpleET.DB.LoggingTable;
import org.dyldr.SimpleET.Entity.LoggingEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

@Aspect
@Component
public class UserActivityAspect {

    @Autowired
    private LoggingTable loggingTable;

//    @Before("execution(* org.dyldr.SimpleET.WebAPI.Order.OrderController.createOrder(..))")
//    public void logUserPurchase(JoinPoint joinPoint) {
//        try {
//            Object[] args = joinPoint.getArgs();
//            var log = new LoggingEntity();
//            log.setAction(LoggingEntity.ActionType.PURCHASE);
//            log.setDetail(java.util.Arrays.toString(args));
//            log.setTimestamp(new java.util.Date());
//            loggingTable.save(log);
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//    }
//
//    @Before("execution(* org.dyldr.SimpleET.WebAPI.Product.ProductController.*(..))")
//    public void logUserProduct(JoinPoint joinPoint) {
//        try{
//            Object[] args = joinPoint.getArgs();
//            var log = new LoggingEntity();
//            log.setAction(LoggingEntity.ActionType.VISIT);
//            log.setDetail(java.util.Arrays.toString(args));
//            log.setTimestamp(new java.util.Date());
//            loggingTable.save(log);
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//    }

    @Pointcut("execution(* org.dyldr.SimpleET.WebAPI.Order.OrderController.createOrder(..))")
    public void createOrderPointcut() {}

    @Pointcut("execution(* org.dyldr.SimpleET.WebAPI.Product.ProductController.*(..))")
    public void productControllerPointcut() {}

    @Before("createOrderPointcut()")
    public void logUserPurchase(JoinPoint joinPoint) {
        logUserActivity(joinPoint, LoggingEntity.ActionType.PURCHASE);
    }

    @Before("productControllerPointcut()")
    public void logUserProduct(JoinPoint joinPoint) {
        logUserActivity(joinPoint, LoggingEntity.ActionType.VISIT);
    }

    private void logUserActivity(JoinPoint joinPoint, LoggingEntity.ActionType actionType) {
        try {
            HttpServletRequest request = ((ServletRequestAttributes) RequestContextHolder.getRequestAttributes()).getRequest();
            String path = request.getRequestURI();

            Object[] args = joinPoint.getArgs();
            var log = new LoggingEntity();
            log.setAction(actionType);
            log.setDetail("Path: " + path + ", Args: " + java.util.Arrays.toString(args));
            log.setTimestamp(new java.util.Date());
            loggingTable.save(log);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}