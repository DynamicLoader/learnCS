package org.dyldr.SimpleET.Entity;

import lombok.Data;
import jakarta.persistence.*;


@Entity
@Data
public class OrderDetailEntity {
    @Id
    @GeneratedValue(strategy=GenerationType.UUID)
    private String id;

    @ManyToOne
    @JoinColumn()
    private OrderEntity order;

    @ManyToOne
    @JoinColumn()
    private ProductEntity product;

    @Column(nullable = false)
    private Integer count = 0;

    @Column(nullable = false)
    private Double price = 0.0;

    @Transient
    public Double getTotalPrice() {
        return count * price;
    }
}
