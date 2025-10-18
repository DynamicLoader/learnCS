package org.dyldr.SimpleET.WebAPI.Payment;

import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/payment/callback")
public class PayCallbackService {
    public record PayRet<T>(int code, T data) {
    }


//    @PostMapping("builtin")
//    public PayRet<String> builtinCallback(
//
//    ) {
//        return new PayRet<>(0, "Success");
//    }

}


