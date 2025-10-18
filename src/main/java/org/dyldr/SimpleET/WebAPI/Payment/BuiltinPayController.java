package org.dyldr.SimpleET.WebAPI.Payment;


import jakarta.websocket.server.PathParam;
import org.dyldr.SimpleET.Entity.OrderEntity;
import org.dyldr.SimpleET.Service.Order.OrderService;
import org.dyldr.SimpleET.Service.Payment.BuiltinPayService;
import org.dyldr.SimpleET.Service.Payment.PayOrderCallbackService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/payment/builtin")
public class BuiltinPayController {

    record BuiltinPayRet<T>(Integer code, T data) {
    }

    record BuiltinPayAccountSummary(Double balance) {
    }

    record BuiltinPayItem(String paymentId, String description, Double total, String createTime) {
    }

    @Autowired
    private BuiltinPayService builtinPayService;

    @Autowired
    private PayOrderCallbackService payOrderCallbackService;

    @Autowired
    private OrderService orderService;


    @GetMapping("/account")
    public BuiltinPayRet<BuiltinPayAccountSummary> getAccountSummary(
            @RequestAttribute Map<String, String> token
    ) {
        String uid = token.get("uid");
        var account = builtinPayService.getAccount(uid);
        if (account == null) {
            return new BuiltinPayRet<>(-1, null);
        }
        return new BuiltinPayRet<>(0, new BuiltinPayAccountSummary(account.getBalance()));
    }

    @PostMapping("/account")
    public BuiltinPayRet<BuiltinPayAccountSummary> createAccount(
            @RequestAttribute Map<String, String> token
    ) {
        String uid = token.get("uid");
        var ret = builtinPayService.createAccount(uid);
        if (ret.code() != 0) {
            return new BuiltinPayRet<>(ret.code(), null);
        }
        return new BuiltinPayRet<>(0, new BuiltinPayAccountSummary(ret.data().getBalance()));
    }



    @PostMapping("/account/recharge")
    public BuiltinPayRet<BuiltinPayItem> recharge(
            @RequestAttribute Map<String, String> token,
            @RequestParam String desc,
            @RequestParam Double total
    ) {
        String uid = token.get("uid");
        try {
            if (total < 0)
                return new BuiltinPayRet<>(-98, null);

            var ret = builtinPayService.modify(uid, desc, total);
            if (ret.code() != 0) {
                return new BuiltinPayRet<>(ret.code(), null);
            }
            return new BuiltinPayRet<>(0, new BuiltinPayItem(
                    ret.data().getId(),
                    ret.data().getDescription(),
                    ret.data().getAmount(),
                    ret.data().getCreateTime().toString())
            );
        } catch (Exception e) {
            return new BuiltinPayRet<>(-99, null);
        }
    }

    @PostMapping("/pay/order/{oid}")
    public BuiltinPayRet<BuiltinPayItem> payOrder(
            @RequestAttribute Map<String, String> token,
            @RequestParam Double total,
            @PathVariable String oid
    ) {
        String uid = token.get("uid");
        try {
            if (total < 0)
                return new BuiltinPayRet<>(-98, null);
            var ord = orderService.getById(oid);
            if (oid == null) {
                return new BuiltinPayRet<>(-100, null);
            }
            if(ord.getStatus() != OrderEntity.OrderStatus.CREATED){
                return new BuiltinPayRet<>(-102,null); // Cannot pay to a paid order
            }
            if(!ord.getTotalPrice().equals(total)){
                return new BuiltinPayRet<>(-101, null); // Paid not satisfy
            }

            var ret = builtinPayService.modify(uid, oid, -total);
            if (ret.code() != 0) {
                return new BuiltinPayRet<>(ret.code(), null);
            }
            var payRet = payOrderCallbackService.pay(ret.data().getId(), oid, PayOrderCallbackService.PayMethod.BUILTIN);
            if (payRet.code() != 0) {
                return new BuiltinPayRet<>(payRet.code(), null);
            }
            return new BuiltinPayRet<>(0, new BuiltinPayItem(
                    ret.data().getId(),
                    ret.data().getDescription(),
                    ret.data().getAmount(),
                    ret.data().getCreateTime().toString())
            );
        } catch (Exception e) {
            return new BuiltinPayRet<>(-99, null);
        }
    }

//    @GetMapping("/record")
//    public BuiltinPayRet<List<BuiltinPayItem>> getAllRecord(
//            @RequestAttribute Map<String, String> token
//    ){
//
//    }

    @GetMapping("/record/{rid}")
    public BuiltinPayRet<BuiltinPayItem> getRecord(
            @PathVariable String rid,
            @RequestAttribute Map<String, String> token
    ) {
        var record = builtinPayService.getRecord(rid);
        if (record == null) {
            return new BuiltinPayRet<>(-1, null);
        }
        if (!record.data().getAccount().getUser().getId().equals(token.get("uid"))) {
            return new BuiltinPayRet<>(-2, null);
        }
        return new BuiltinPayRet<>(0, new BuiltinPayItem(
                record.data().getId(),
                record.data().getDescription(),
                record.data().getAmount(),
                record.data().getCreateTime().toString())
        );
    }

}
