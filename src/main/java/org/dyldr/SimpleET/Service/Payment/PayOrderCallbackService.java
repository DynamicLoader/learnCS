package org.dyldr.SimpleET.Service.Payment;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.DB.BuiltinPayRecordTable;
import org.dyldr.SimpleET.DB.OrderTable;
import org.dyldr.SimpleET.Entity.BuiltinPayRecordEntity;
import org.dyldr.SimpleET.Entity.OrderEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class PayOrderCallbackService {

    @Autowired
    BuiltinPayRecordTable builtinPayRecordTable;

    @Autowired
    OrderTable orderTable;

    public record PayRet<T>(int code, T data) {
    }

    public enum PayMethod {
        BUILTIN,
//        ALIPAY,
//        WECHATPAY
    }

    ;

    @Transactional
    public PayRet<String> pay(String payId, String oid, PayMethod method) {
        switch (method) {
            case BUILTIN:
                return builtinPay(payId, oid);
//            case ALIPAY:
//                return alipay(payId, oid);
//            case WECHATPAY:
//                return wechatpay(payId, oid);
            default:
                return new PayRet<>(-99, "Unknown method");
        }
    }

    private PayRet<String> builtinPay(String payId, String oid) {
        var order = orderTable.findById(oid).orElse(null);
        if (order == null) {
            return new PayRet<>(-1, "Order not found");
        }
        order.setStatus(OrderEntity.OrderStatus.PAID);
        var bpay = new BuiltinPayRecordEntity();
        bpay.setId(payId);
        order.setBuiltinPay(bpay);
        orderTable.save(order);
        return new PayRet<>(0, "Success");
    }
}
