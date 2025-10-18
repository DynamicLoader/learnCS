package org.dyldr.SimpleET.WebAPI.Product;


import org.dyldr.SimpleET.Entity.ProductEntity;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import org.dyldr.SimpleET.Service.Product.ProductListService;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/product")
public class ProductController {

    record ProductItem(String pid, String name, String sellerName, double price, double expressFee, String image, Integer totalSold, Integer stock) {
    }

    record ProductRet<T>(int code, T data) {
    }

    record ProductDetail(String pid, String name, String description, double price, int stock, String category,
                         String image, String seller, double expressFee, ProductEntity.Status status, Integer totalSold) {
    }


    @Autowired
    private ProductListService prdListSrv;

    @GetMapping("")
    public ProductRet<List<ProductItem>> search(
            @RequestParam(value = "name", defaultValue = "") String name,
            @RequestParam(value = "category", defaultValue = "") String category,
            @RequestParam(value = "sellerName", defaultValue = "") String sellerName,
            // Below are Additional Parameters
            @RequestParam(value = "minPrice", defaultValue = "0") double minPrice,
            @RequestParam(value = "maxPrice", defaultValue = "1000000000") double maxPrice,
            @RequestParam(value = "maxExpressFee", defaultValue = "1000000000") double maxExpressFee,

            @RequestAttribute(value = "token") Map<String, String> token
    ) {
        List<ProductEntity> ret = prdListSrv.searchBy(name, category, sellerName, minPrice, maxPrice, maxExpressFee);
        List<ProductItem> i = new ArrayList<ProductItem>(ret.size());
        for (ProductEntity p : ret) {
            if(p.getStatus() == ProductEntity.Status.DELETED)
                continue;
            if (p.getStatus() != ProductEntity.Status.ON_SALE && !token.get("uid").equals(p.getSeller().getId()) && !token.get("role").equals("ADMIN"))
                continue; // public can only see on sale products
            i.add(new ProductItem(p.getId(), p.getName(), p.getSeller().getName(), p.getPrice(), p.getExpressFee(), p.getImage(), p.getTotalSold(), p.getStock()));
        }
        return new ProductRet<List<ProductItem>>(0, i);
    }

    @PostMapping("")
    public ProductRet<String> addProduct(
            @RequestParam(value = "name") String name,
            @RequestParam(value = "description", defaultValue = "") String description,
            @RequestParam(value = "price") double price,
            @RequestParam(value = "stock") int stock,
            @RequestParam(value = "category") String category,
            @RequestParam(value = "image", defaultValue = "") String image,
            @RequestParam(value = "expressFee") double expressFee,
            @RequestParam(value = "status", defaultValue = "OFF_SALE") ProductEntity.Status status,
            @RequestAttribute(value = "token") Map<String, String> token
    ) {
        try {
            String seller = token.get("uid");
            if (!UserEntity.UserRole.fromString(token.get("role")).canModifyProduct()) {
                return new ProductRet<>(-1, "No Permission");
            }

            ProductEntity prd = prdListSrv.addProduct(name, description, price, stock, category, image, seller, expressFee, status);
            return new ProductRet<>(0, prd.getId());
        } catch (Exception e) {
            return new ProductRet<>(-2, "Invalid data");
        }
    }

    @DeleteMapping("{pid}")
    public ProductRet<String> deleteProduct(
            @PathVariable String pid,
            @RequestAttribute Map<String, String> token
    ) {
        String seller = token.get("uid");
        UserEntity.UserRole role = UserEntity.UserRole.fromString(token.get("role"));
        if (!role.canModifyProduct()) {
            return new ProductRet<String>(-2, "No Permission");
        }
        try {
            if (prdListSrv.deleteProduct(pid, seller) ||
                    (role.canModifyProductSuper() && prdListSrv.deleteProduct(pid))
            ) {
                return new ProductRet<String>(0, "Success");
            }

        } catch (Exception e) {
            return new ProductRet<String>(-1, "Internal error");
        }
        return new ProductRet<String>(-3, "No such product");
    }

    @GetMapping("{pid}")
    public ProductRet<ProductDetail> getProduct(
            @PathVariable String pid,

            @RequestAttribute Map<String, String> token
    ) {
        try {
            ProductEntity ret = prdListSrv.getProduct(pid);
            if (ret == null) {
                return new ProductRet<ProductDetail>(-2, null); // No such product
            }
            if (ret.getStatus() != ProductEntity.Status.ON_SALE && !token.get("uid").equals(ret.getSeller().getId()) && !token.get("role").equals("ADMIN")) {
                return new ProductRet<ProductDetail>(-3, null); // Not on sale
            }
            if (ret.getStatus().equals(ProductEntity.Status.DELETED)) {
                return new ProductRet<ProductDetail>(-4, null); // Deleted
            }
            return new ProductRet<ProductDetail>(0, new ProductDetail(
                    ret.getId(),
                    ret.getName(),
                    ret.getDescription(),
                    ret.getPrice(),
                    ret.getStock(),
                    ret.getCategory(),
                    ret.getImage(),
                    ret.getSeller().getName(),
                    ret.getExpressFee(),
                    ret.getStatus(),
                    ret.getTotalSold()
            ));
        } catch (Exception e) {
            return new ProductRet<ProductDetail>(-1, null);
        }
    }

    @PutMapping("{pid}")
    public ProductRet<String> updateProduct(
            @PathVariable String pid,
            @RequestParam(value = "name", defaultValue = "") String name,
            @RequestParam(value = "description", defaultValue = "") String description,
            @RequestParam(value = "price", defaultValue = "0") double price,
            @RequestParam(value = "stock", defaultValue = "0") int stock,
            @RequestParam(value = "category", defaultValue = "") String category,
            @RequestParam(value = "image", defaultValue = "") String image,
            @RequestParam(value = "expressFee", defaultValue = "0") double expressFee,
            @RequestParam(value = "status", defaultValue = "OFF_SALE") ProductEntity.Status status,
            @RequestAttribute Map<String, String> token
    ) {
        String seller = token.get("uid");
        UserEntity.UserRole role = UserEntity.UserRole.fromString(token.get("role"));
        if (!role.canModifyProduct()) {
            return new ProductRet<String>(-2, "No Permission");
        }
        try {
            prdListSrv.updateProduct(pid, name, description, price, stock, category, image, seller, expressFee, status);
            return new ProductRet<String>(0, "Success");
        } catch (Exception e) {
            return new ProductRet<String>(-1, "Invalid data");
        }
//        return new ProductRet<String>(-3, "No such product");
    }
}
