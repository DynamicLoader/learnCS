package org.dyldr.SimpleET.WebAPI;

import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.boot.web.servlet.error.ErrorController;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;


@Controller
public class SpaErrorController implements ErrorController {

    @RequestMapping("/error")
    public Object error(HttpServletRequest request, HttpServletResponse response) {
        // place your additional code here (such as error logging...)
        if (request.getMethod().equalsIgnoreCase(HttpMethod.GET.name())) {
            response.setStatus(HttpStatus.OK.value()); // optional.
            return "forward:/index.html"; // forward to static SPA html resource.
        } else {
            return ResponseEntity.notFound().build(); // or your REST 404 blabla...
        }
    }

//    @Override
//    public String getErrorPath() {
//        return "/error";
//    }
}