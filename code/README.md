# Results

##  Slowdown factor comparison

The following chart displays the slowdown factor of Deep Neural Networks models accelerated on Versal AI Engine in presence of contention on shared resources.

![Slowdown factor comparison](img/slowdown_factor_comparison.png)

Among the models listed, SqueezeNet demonstrates the highest slowdown factor, approximately 35 times slower compared to the baseline. This means that SqueezeNet experiences a significant degradation in performance when accessing shared resources, particularly due to its lightweight architecture. SqueezeNet has the fewest parameters, only 5 MB, among all the models considered.

On the other hand, AgeNet, which is a custom VGG16-based network, exhibits a slowdown factor of around 4. This implies that AgeNet's performance is less affected by resource access delays compared to SqueezeNet but still experiences a noticeable slowdown.

The distinction between the two types of memory bombs, one with priority given to the scheduler and the other without, does not show any significant differences in terms of slowdown. This suggests that the scheduling priority does not have a substantial impact on the observed slowdown factors.

In summary, the dataframe showcases the relative slowdown factors experienced by different neural network models in the Versal VCK190 system when confronted with memory-intensive tasks. SqueezeNet, being the lightest model, is most affected, experiencing a slowdown of approximately 35 times, while AgeNet, a heavier model, exhibits a slowdown of around 4 times. The scheduler's priority in memory access does not seem to influence the slowdown significantly.



##  Age detection model

![Age Model](img/age_model.png)



##  ResNet50 model

![ResNet50 Model](img/resnet_model.png)



##  SqueezeNet model

![SqueezeNet Model](img/squeezenet_model.png)